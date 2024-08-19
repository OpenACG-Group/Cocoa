/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include <future>
#include <chrono>

#include "include/core/SkPixmap.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/core/SkColorSpace.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/vk/VulkanMutableTextureState.h"

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VULKAN

#include "CRPKG/ResourceManager.h"
#include "CRPKG/VirtualDisk.h"
#include "Core/EventLoop.h"
#include "Core/Journal.h"
#include "Core/TraceEvent.h"
#include "Glamor/HWComposeContext.h"
#include "Glamor/HWComposeDevice.h"
#include "Glamor/Layers/ExternalTextureLayer.h"
#include "Glamor/SkiaGpuContextOwner.h"
#include "Utau/FrameTextureConverter.h"

#include "HWDeviceContext.h"
#include "Utau/PixelFormatUtils.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.FrameTextureConverter)

namespace
{

constexpr VkFormat kNV12VkFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;

sk_sp<SkImage> borrow_vulkan_frame_to_SkImage(gl::SkiaGpuContextOwner *gpu_ctx, const AVFrame *frame)
{
    CHECK(gpu_ctx && frame->format == AV_PIX_FMT_VULKAN);
    const AVVkFrame *vkframe = reinterpret_cast<const AVVkFrame*>(frame->data[0]);
    AVHWFramesContext *hwframes_ctx = reinterpret_cast<AVHWFramesContext*>(frame->hw_frames_ctx->data);
    AVVulkanFramesContext *hw_vk_frames_ctx = static_cast<AVVulkanFramesContext*>(hwframes_ctx->hwctx);
    AVVulkanDeviceContext *hw_vk_device_ctx = static_cast<AVVulkanDeviceContext*>(hwframes_ctx->device_ctx->hwctx);
    VkDevice vk_device = hw_vk_device_ctx->act_dev;

    // The video frame must use the same Vulkan logical device to our GPU context.
    // Since FFmpeg does not expose the memory type index in `AVVkFrame` interface, which is
    // required for importing the external memory contents from a different Vulkan logical device,
    // zerocopy texture sharing is impossible.
    if (vk_device != gpu_ctx->GetVkDevice())
        return nullptr;

    size_t nb_textures = std::ranges::count_if(vkframe->img, [](VkImage handle) {
        return handle != VK_NULL_HANDLE;
    });

    if (hwframes_ctx->sw_format == AV_PIX_FMT_NV12 && nb_textures == 1 &&
        hw_vk_frames_ctx->format[0] == kNV12VkFormat)
    {
        VkFormatProperties format_props;
        vkGetPhysicalDeviceFormatProperties(hw_vk_device_ctx->phys_dev, kNV12VkFormat, &format_props);
        VkFormatFeatureFlags flags;
        if (vkframe->tiling == VK_IMAGE_TILING_OPTIMAL)
            flags = format_props.optimalTilingFeatures;
        else if (vkframe->tiling == VK_IMAGE_TILING_LINEAR)
            flags = format_props.linearTilingFeatures;
        else
            return nullptr;

        if (!(flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) ||
            !(flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT) ||
            !(flags & VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) ||
            !(flags & VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT))
        {
            // Physical device does not support YUV sampler on NV12 format with current tiling
            return nullptr;
        }

        VkSamplerYcbcrModelConversion ycbcr_model;
        switch (frame->colorspace)
        {
        case AVCOL_SPC_UNSPECIFIED:
        case AVCOL_SPC_BT709:
            ycbcr_model = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709;
            break;
        case AVCOL_SPC_BT470BG:
            ycbcr_model = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601;
            break;
        case AVCOL_SPC_BT2020_NCL:
            ycbcr_model = VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020;
            break;
        default:
            // Unsupported YUV colorspace
            return nullptr;
        }

        VkSamplerYcbcrRange ycbcr_range;
        switch (frame->color_range)
        {
        case AVCOL_RANGE_UNSPECIFIED:
        case AVCOL_RANGE_MPEG:
            ycbcr_range = VK_SAMPLER_YCBCR_RANGE_ITU_NARROW;
            break;
        case AVCOL_RANGE_JPEG:
            ycbcr_range = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
            break;
        default:
            MARK_UNREACHABLE();
        }

        GrVkYcbcrConversionInfo conversion_info{
            .fFormat = kNV12VkFormat,
            .fExternalFormat = 0,
            .fYcbcrModel = ycbcr_model,
            .fYcbcrRange = ycbcr_range,
            .fXChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN,
            .fYChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN,
            .fChromaFilter = VK_FILTER_LINEAR,
            .fForceExplicitReconstruction = false,
            .fFormatFeatures = flags
        };

        skgpu::VulkanAlloc vk_alloc_info;
        vk_alloc_info.fMemory = vkframe->mem[0];
        vk_alloc_info.fOffset = vkframe->offset[0];
        vk_alloc_info.fSize = vkframe->size[0];

        GrVkImageInfo vk_image_info{
            .fImage = vkframe->img[0],
            .fAlloc = vk_alloc_info,
            .fImageTiling = vkframe->tiling,
            // FFmpeg gives us a VkImage that has layout `VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR`, however
            // image layout transition is not needed here, since we have told Skia the current image layout
            // and Skia will do the image layout transition internally.
            .fImageLayout = vkframe->layout[0],
            .fFormat = kNV12VkFormat,
            .fImageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT
                              | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                              | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            .fSampleCount = 1,
            .fLevelCount = 1,
            // Queue ownership transition is also not needed, since FFmpeg created the VkImage with the
            // sharing mode CONCURRENT.
            .fCurrentQueueFamily = gpu_ctx->GetSkiaQueueFamilyIndex(),
            .fProtected = GrProtected::kNo,
            .fYcbcrConversionInfo = conversion_info,
            .fSharingMode = VK_SHARING_MODE_CONCURRENT
        };

        GrBackendTexture vk_texture = GrBackendTextures::MakeVk(
            frame->width, frame->height, vk_image_info, "nv12_video_texture");
        if (!vk_texture.isValid())
            return nullptr;

        sk_sp<SkColorSpace> color_space = ResolveFrameColorCharacteristicsToSkColorSpace(
            frame->color_primaries, frame->color_trc, SkColorSpace::MakeSRGB());

        // Clone to get the ownership of the underlying resources
        AVFrame *frame_clone = av_frame_clone(frame);
        sk_sp<SkImage> image = SkImages::BorrowTextureFrom(
            gpu_ctx->GetSkiaGpuContext(),
            vk_texture,
            kTopLeft_GrSurfaceOrigin,
            kRGB_888x_SkColorType,
            kPremul_SkAlphaType,
            color_space,
            +[](void *userdata) {
                // This callback will be called when the texture is not needed anymore.
                // If this is the last reference to the frame, the underlying buffer will be returned
                // to the pool, then the decoder can reuse that buffer for decoding.
                AVFrame *frame = static_cast<AVFrame*>(userdata);
                av_frame_free(&frame);
            },
            frame_clone
        );

        return image;
    }

    return nullptr;
}

class ExternalTextureAccessorImpl : public gl::ExternalTextureAccessor
{
public:
    ExternalTextureAccessorImpl(AVFrame *frame, const SkISize& dimensions, ScaleResampler resampler)
        : frame_(av_frame_clone(frame)), dimensions_(dimensions), resampler_(resampler) {}
    ~ExternalTextureAccessorImpl() override
    {
        if (frame_)
            av_frame_free(&frame_);
    }

    void Prefetch() override {}
    void Release() override
    {
        if (frame_)
            av_frame_free(&frame_);
    }

    sk_sp<SkImage> Acquire(gl::SkiaGpuContextOwner *gpu_ctx, const SkColorInfo& preferred_color_info) override
    {
        TRACE_EVENT("multimedia", "ExternalTextureAccessorImpl::Acquire");

        // Try GPU texture sharing (zero copy) if available
        if (gpu_ctx && frame_->format == AV_PIX_FMT_VULKAN)
        {
            if (sk_sp<SkImage> image = borrow_vulkan_frame_to_SkImage(gpu_ctx, frame_))
                return image;
        }

        // If GPU texture sharing is unavailable or impossible, and the frame is hardware frame,
        // we download it into CPU memory first.
        const AVPixFmtDescriptor *format_desc = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(frame_->format));
        if (format_desc->flags & AV_PIX_FMT_FLAG_HWACCEL)
        {
            AVFrame *dst_frame = av_frame_alloc();
            if (int ret = av_hwframe_transfer_data(dst_frame, frame_, 0); ret < 0)
            {
                QLOG(LOG_ERROR, "Could not acquire AVFrame pixels: hwframe transfer error: {}", av_err2str(ret));
                return nullptr;
            }
            av_frame_free(&frame_);
            frame_ = dst_frame;
        }

        // If GPU context is available, uploads the YUV texture into GPU memory and
        // YUV => RGB conversion will be done by GPU.
        if (gpu_ctx)
        {
            SkYUVAPixmaps pixmaps = WrapFrameToSkYUVAPixmaps(frame_);
            if (pixmaps.isValid())
            {
                return SkImages::TextureFromYUVAPixmaps(
                    gpu_ctx->GetSkiaGpuContext(), pixmaps,
                    skgpu::Mipmapped::kNo, false,
                    ResolveFrameColorCharacteristicsToSkColorSpace(
                        frame_->color_primaries, frame_->color_trc, SkColorSpace::MakeSRGB()
                    )
                );
            }
        }

        // Now texture sharing is impossible, and GPU format conversion is unavailable.

        const AVPixelFormat format = static_cast<AVPixelFormat>(frame_->format);
        const AVPixFmtDescriptor *fmt_desc = av_pix_fmt_desc_get(format);
        SkAlphaType alpha_type = kOpaque_SkAlphaType;
        if (fmt_desc->flags & AV_PIX_FMT_FLAG_ALPHA)
            alpha_type = kUnpremul_SkAlphaType;

        SkImageInfo image_info = SkImageInfo::Make(
            dimensions_,
            preferred_color_info.colorType(),
            alpha_type,
            preferred_color_info.refColorSpace()
        );
        return BlitFrameToSkImage(
            frame_,
            SkIRect::MakeSize(dimensions_),
            image_info,
            resampler_
        );
    }

private:
    AVFrame             *frame_;
    SkISize              dimensions_;
    ScaleResampler       resampler_;
};

} // namespace anonymous

std::unique_ptr<gl::ExternalTextureLayer>
WrapFrameToGLExternalLayer(AVFrame *frame, const SkPoint& offset,
                           const SkISize& dimensions, ScaleResampler resampler)
{
    SkSamplingOptions sk_sampling;
    switch (resampler)
    {
    case utau::ScaleResampler::kNearest:
        sk_sampling = SkSamplingOptions(SkFilterMode::kNearest);
        break;
    case utau::ScaleResampler::kBilinear:
        sk_sampling = SkSamplingOptions(SkFilterMode::kLinear);
        break;
    case utau::ScaleResampler::kBicubic:
        sk_sampling = SkCubicResampler::Mitchell();
        break;
    default:
        MARK_UNREACHABLE();
    }

    auto accessor = std::make_unique<ExternalTextureAccessorImpl>(frame, dimensions, resampler);
    return std::make_unique<gl::ExternalTextureLayer>(std::move(accessor), offset, dimensions, sk_sampling);
}

UTAU_NAMESPACE_END
