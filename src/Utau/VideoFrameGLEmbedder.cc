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
#include "include/core/SkImage.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/core/SkColorSpace.h"

#define FFWRAP_AVUTIL_USE_HWCONTEXT_VAAPI

#include "Core/EventLoop.h"
#include "Core/Journal.h"
#include "Core/TraceEvent.h"
#include "Glamor/Layers/ExternalTextureLayer.h"
#include "Utau/VideoFrameGLEmbedder.h"
#include "Utau/VideoBuffer.h"
#include "Utau/ffwrappers/libswscale.h"
#include "Utau/ffwrappers/libavutil.h"
UTAU_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Utau.VideoFrameGLEmbedder)

struct SwsContextParam
{
    SwsContextParam()
        : src_format(AV_PIX_FMT_NB)
        , dst_format(AV_PIX_FMT_NB)
        , srcw(0)
        , srch(0)
        , dstw(0)
        , dsth(0) {}

    AVPixelFormat   src_format;
    AVPixelFormat   dst_format;
    int32_t         srcw;
    int32_t         srch;
    int32_t         dstw;
    int32_t         dsth;
    SkSamplingOptions sampling;

    g_nodiscard bool IsValid() const {
        return (src_format != AV_PIX_FMT_NB && dst_format != AV_PIX_FMT_NB &&
                srcw > 0 && srch > 0 && dstw > 0 && dsth > 0);
    }

    bool operator==(const SwsContextParam& p) const {
        if (!p.IsValid() || !IsValid())
            return false;

        return (p.src_format == src_format &&
                p.dst_format == dst_format &&
                p.srcw == srcw &&
                p.srch == srch &&
                p.dstw == dstw &&
                p.dsth == dsth &&
                p.sampling == sampling);
    }
};

struct SwsContextParamPair
{
    SwsContextParamPair()
        : context(nullptr), param() {}

    ~SwsContextParamPair() {
        if (context)
            sws_freeContext(context);
    }

    SwsContext          *context = nullptr;
    SwsContextParam      param;
};

struct VideoFrameGLEmbedder::SwscaleContextCache
{
    constexpr static int kMaxContexts = 4;

    SwscaleContextCache() : override_idx(0) {}

    std::array<SwsContextParamPair, kMaxContexts> contexts;
    int override_idx;

#define ITERATOR_PAIR(x) x.begin(), x.end()

    SwsContext *UpdateContext(const SwsContextParam& param)
    {
        for (const auto& entry : contexts)
        {
            if (entry.context && entry.param == param)
                return entry.context;
        }

        SwsContextParamPair *newpair = nullptr;

        auto free_pair_itr = std::find_if(ITERATOR_PAIR(contexts), [](const auto& pair) {
            return !pair.context;
        });
        if (free_pair_itr == contexts.end())
        {
            // Not found, which means the number of contexts have exceeded
            // the limitation, then override a context.
            newpair = &contexts[override_idx];
            sws_freeContext(newpair->context);
            newpair->context = nullptr;
            newpair->param = {};
            override_idx = (override_idx + 1) % kMaxContexts;
        }
        else
            newpair = &(*free_pair_itr);

        SwsContext *c = sws_alloc_context();
        CHECK(c && "Failed to allocate memory");

        av_opt_set_pixel_fmt(c, "src_format", param.src_format, 0);
        av_opt_set_pixel_fmt(c, "dst_format", param.dst_format, 0);
        av_opt_set_int(c, "srcw", param.srcw, 0);
        av_opt_set_int(c, "srch", param.srch, 0);
        av_opt_set_int(c, "dstw", param.dstw, 0);
        av_opt_set_int(c, "dsth", param.dsth, 0);

        if (param.sampling.useCubic)
        {
            av_opt_set(c, "sws_flags", "bicubic", 0);
            av_opt_set_double(c, "param0", param.sampling.cubic.B, 0);
            av_opt_set_double(c, "param1", param.sampling.cubic.C, 0);
        }
        else if (param.sampling.filter == SkFilterMode::kNearest)
        {
            av_opt_set(c, "sws_flags", "neighbor", 0);
        }
        else if (param.sampling.filter == SkFilterMode::kLinear)
        {
            av_opt_set(c, "sws_flags", "bilinear", 0);
        }

        int ret = sws_init_context(c, nullptr, nullptr);
        if (ret < 0)
        {
            sws_freeContext(c);
            QLOG(LOG_ERROR, "Failed to initialize swscale context: {}", av_err2str(ret));
            return nullptr;
        }

        newpair->param = param;
        newpair->context = c;

        return c;
    }

#undef ITERATOR_PAIR
};

using SwscaleContextCache = VideoFrameGLEmbedder::SwscaleContextCache;

namespace {

sk_sp<SkImage> create_skimage_from_rgb_frame(GrDirectContext *direct, AVFrame *frame,
                                             bool copy_pixels)
{
    SkColorType color_type = kBGRA_8888_SkColorType;
    SkAlphaType alpha_type;
    if (frame->format == AV_PIX_FMT_BGRA)
        alpha_type = SkAlphaType::kUnpremul_SkAlphaType;
    else if (frame->format == AV_PIX_FMT_BGR0)
        alpha_type = SkAlphaType::kOpaque_SkAlphaType;
    else
    {
        QLOG(LOG_ERROR, "Unsupported color format to create a RGB image");
        return nullptr;
    }

    AVFrame *frame_dup = av_frame_clone(frame);
    CHECK(frame_dup && "Failed to allocate memory");

    SkImageInfo info = SkImageInfo::Make(SkISize::Make(frame_dup->width, frame_dup->height),
                                         color_type,
                                         alpha_type,
                                         SkColorSpace::MakeSRGB());

    SkPixmap pixmap(info, frame_dup->data[0], frame_dup->linesize[0]);

    sk_sp<SkImage> raster_image;
    if (copy_pixels)
    {
        raster_image = SkImage::MakeRasterCopy(pixmap);
    }
    else
    {
        // Pixels are shared with `SkImage` without any copy
        raster_image = SkImage::MakeFromRaster(pixmap,
                                               [](const void *pixels, void *closure) {
                                                   CHECK(pixels && closure);
                                                   auto *frame = reinterpret_cast<AVFrame*>(closure);
                                                   av_frame_free(&frame);
                                               }, frame_dup);
    }

    if (!raster_image)
    {
        QLOG(LOG_ERROR, "Could not create a raster-backed image from AVFrame");
        return nullptr;
    }

    if (!direct)
        return raster_image;

    sk_sp<SkImage> texture_image = raster_image->makeTextureImage(
            direct, GrMipmapped::kNo, SkBudgeted::kYes);
    if (!texture_image)
        QLOG(LOG_ERROR, "Could not create a GPU-backed texture image");

    return texture_image;
}

AVFrame *convert_frame_to_rgb(AVFrame *frame,
                              const SkISize& scale_size,
                              const SkSamplingOptions& sampling,
                              SwscaleContextCache *sws_ctx_cache)
{
    SwsContextParam param{};
    param.src_format = static_cast<AVPixelFormat>(frame->format);
    param.dst_format = AV_PIX_FMT_BGRA;
    param.srcw = frame->width;
    param.srch = frame->height;
    param.dstw = scale_size.width();
    param.dsth = scale_size.height();
    param.sampling = sampling;

    SwsContext *ctx = sws_ctx_cache->UpdateContext(param);
    if (!ctx)
        return nullptr;

    AVFrame *dst = av_frame_alloc();
    CHECK(dst && "Failed to allocate memory");

    int ret = sws_scale_frame(ctx, dst, frame);
    if (ret < 0)
    {
        av_frame_free(&dst);
        QLOG(LOG_ERROR, "Failed to scale frame: {}", av_err2str(ret));
    }

    return dst;
}

struct FrameYUVAInfoMapEntry
{
    AVPixelFormat av_fmt;
    SkYUVAInfo::PlaneConfig sk_plane_config;
    SkYUVAInfo::Subsampling sk_subsampling;
} const g_frame_yuvainfo_map[] = {
#define P SkYUVAInfo::PlaneConfig
#define S SkYUVAInfo::Subsampling
    { AV_PIX_FMT_YUV420P, P::kY_U_V, S::k420 },
    { AV_PIX_FMT_YUV422P, P::kY_U_V, S::k422 },
    { AV_PIX_FMT_YUV444P, P::kY_U_V, S::k444 },
    { AV_PIX_FMT_YUV410P, P::kY_U_V, S::k410 },
    { AV_PIX_FMT_YUV411P, P::kY_U_V, S::k411 },
    { AV_PIX_FMT_NV12,    P::kY_UV,  S::k420 },
    { AV_PIX_FMT_NV21,    P::kY_VU,  S::k420 },
    { AV_PIX_FMT_NV24,    P::kY_UV,  S::k444 },
    { AV_PIX_FMT_NV42,    P::kY_VU,  S::k444 }
#undef P
#undef S
};

struct FrameYUVAColorspaceMapEntry
{
    AVColorTransferCharacteristic av_ctc;
    AVColorRange av_range;
    AVColorSpace av_colorspace;
    SkYUVColorSpace sk_colorspace;
} const g_frame_yuva_colorspace_map[] = {
    { AVCOL_TRC_UNSPECIFIED, AVCOL_RANGE_UNSPECIFIED, AVCOL_SPC_UNSPECIFIED, kRec709_Limited_SkYUVColorSpace },
    { AVCOL_TRC_BT709, AVCOL_RANGE_MPEG, AVCOL_SPC_BT709, kRec709_Limited_SkYUVColorSpace },
    { AVCOL_TRC_BT709, AVCOL_RANGE_JPEG, AVCOL_SPC_BT709, kRec709_Full_SkYUVColorSpace }
    // TODO(sora): support other formats
};

bool is_skia_supported_yuv_format(int format)
{
    return std::any_of(std::begin(g_frame_yuvainfo_map),
                       std::end(g_frame_yuvainfo_map),
                       [format](const FrameYUVAInfoMapEntry& entry) {
                            return (format == entry.av_fmt);
                       });
}

bool is_skia_supported_rgb_format(int format)
{
    return (format == AV_PIX_FMT_BGRA || format == AV_PIX_FMT_BGR0);
}

SkYUVAPixmapInfo create_yuva_pixmap_info_from_frame(AVFrame *frame)
{
    SkYUVAInfo::PlaneConfig plane_config = SkYUVAInfo::PlaneConfig::kUnknown;
    SkYUVAInfo::Subsampling subsampling = SkYUVAInfo::Subsampling::kUnknown;

    for (const auto& entry : g_frame_yuvainfo_map)
    {
        if (entry.av_fmt == frame->format)
        {
            plane_config = entry.sk_plane_config;
            subsampling = entry.sk_subsampling;
            break;
        }
    }
    if (plane_config == SkYUVAInfo::PlaneConfig::kUnknown)
    {
        QLOG(LOG_ERROR, "Unsupported YUV format or not a YUV format");
        return {};
    }

    SkYUVColorSpace colorspace;
    bool found = false;
    for (const auto& entry : g_frame_yuva_colorspace_map)
    {
        if (frame->color_trc == entry.av_ctc &&
            frame->color_range == entry.av_range &&
            frame->colorspace == entry.av_colorspace)
        {
            colorspace = entry.sk_colorspace;
            found = true;
            break;
        }
    }
    if (!found)
    {
        QLOG(LOG_ERROR, "Unsupported YUV color parameters");
        return {};
    }

    SkYUVAInfo info(SkISize::Make(frame->width, frame->height),
                    plane_config,
                    subsampling,
                    colorspace,
                    SkEncodedOrigin::kTopLeft_SkEncodedOrigin,
                    SkYUVAInfo::Siting::kCentered,
                    SkYUVAInfo::Siting::kCentered);

    static_assert(SkYUVAPixmapInfo::kMaxPlanes <= AV_NUM_DATA_POINTERS);
    size_t row_bytes[SkYUVAPixmapInfo::kMaxPlanes];
    for (int32_t i = 0; i < SkYUVAPixmapInfo::kMaxPlanes; i++)
        row_bytes[i] = frame->linesize[i];

    return {info, SkYUVAPixmapInfo::DataType::kUnorm8, row_bytes};
}

SkYUVAPixmaps create_yuva_pixmaps_from_frame(AVFrame *frame)
{
    SkYUVAPixmapInfo info = create_yuva_pixmap_info_from_frame(frame);
    if (!info.isValid())
        return {};

    int32_t nb_planes = info.numPlanes();

    SkPixmap pixmaps[SkYUVAPixmaps::kMaxPlanes];
    for (int32_t i = 0; i < nb_planes; i++)
    {
        CHECK(frame->data[i]);
        pixmaps[i].reset(info.planeInfo(i), frame->data[i], frame->linesize[i]);
    }

    return SkYUVAPixmaps::FromExternalPixmaps(info.yuvaInfo(), pixmaps);
}

sk_sp<SkImage> create_skimage_gpu_from_yuv_frame(GrDirectContext *direct, AVFrame *frame)
{
    CHECK(direct);
    CHECK(frame);

    SkYUVAPixmaps pixmaps = create_yuva_pixmaps_from_frame(frame);
    if (!pixmaps.isValid())
        return nullptr;

    return SkImage::MakeFromYUVAPixmaps(direct, pixmaps, GrMipmapped::kNo, false);
}

sk_sp<SkImage> create_skimage_raster_from_frame(AVFrame *frame,
                                                const SkISize& scale_size,
                                                const SkSamplingOptions& sampling,
                                                SwscaleContextCache *sws_ctx_cache)
{
    CHECK(frame);

    AVFrame *converted = convert_frame_to_rgb(frame, scale_size, sampling, sws_ctx_cache);
    if (!converted)
        return nullptr;

    sk_sp<SkImage> image = create_skimage_from_rgb_frame(nullptr, converted, false);

    av_frame_free(&converted);
    return image;
}

sk_sp<SkImage> create_skimage_from_frame(GrDirectContext *direct,
                                         AVFrame *frame,
                                         const SkISize& scale_size,
                                         const SkSamplingOptions& sampling,
                                         SwscaleContextCache *sws_ctx_cache,
                                         bool is_frame_mapped_from_hw)
{
    if (is_skia_supported_yuv_format(frame->format) && direct)
    {
        // GPU context is available, then the frame will be scaled
        // by Skia itself (generate an original-sized image, and scaling
        // will be performed by SkCanvas::drawImageRect).
        return create_skimage_gpu_from_yuv_frame(direct, frame);
    }

    if (is_skia_supported_rgb_format(frame->format))
    {
        // The frame will be scaled by swscale, no format conversion
        // is needed.
        return create_skimage_from_rgb_frame(direct, frame, is_frame_mapped_from_hw);
    }

    // Frame should be scaled by swscale and convert to RGB format
    return create_skimage_raster_from_frame(frame, scale_size,
                                            sampling, sws_ctx_cache);
}

class VAAPIVBOAccessor : public gl::ExternalTextureAccessor
{
public:
    VAAPIVBOAccessor(VideoFrameGLEmbedder *embedder,
                     AVFrame *frame,
                     const SkISize& scale_size,
                     const SkSamplingOptions& sampling)
        : embedder_(embedder)
        , map_frame_(nullptr)
        , scale_size_(scale_size)
        , sampling_(sampling)
        , async_map_pending_(true)
        , async_map_promise_(std::make_shared<std::promise<AVFrame*>>())
    {
        CHECK(embedder && frame);

        AVFrame *frame_dup = av_frame_clone(frame);

        /**
         * Mapping a hardware frame is relatively slow for rendering thread.
         * Perform that task in threadpool asynchronously for less overhead
         * on rendering thread. However, we cannot perform YUV->RGB conversion
         * in threadpool if GPU backed `SkImage` is required, as GPU related
         * operations must be executed on rendering thread.
         */
        auto async_executor = [promise = async_map_promise_, frame_dup]() {
            AVFrame *frame = frame_dup;
            AVFrame *map_frame = av_frame_alloc();
            CHECK(map_frame && "Failed to allocate memory");

            TRACE_EVENT("multimedia", "VAAPIVBOAccessor:av_hwframe_map");
            int ret = av_hwframe_map(map_frame, frame, AV_HWFRAME_MAP_READ);

            /**
             * Note:
             *   We should free a VAAPI backed AVFrame as soon as possible.
             *   When hardware-accelerated decoding is enabled, decoder prefers to create
             * a "memory pool" where several GPU surfaces, which will be used while decoding,
             * are allocated in advance. When user requests for next frame, decoder will
             * attempt to find a free surface in the pool to use, and it will report
             * an error if there are no any free surfaces.
             *   The problem is that only a surface which is not referenced by any other frames
             * can be treated as a free surface. If we hold references of those surfaces for a long
             * time, all the surfaces in the pool will become unavailable for decoder.
             */
            av_frame_free(&frame);

            if (ret < 0)
            {
                av_frame_free(&map_frame);
                QLOG(LOG_ERROR, "Failed to map hardware frame: {}", av_err2str(ret));
                promise->set_value(nullptr);
                return;
            }

            promise->set_value(map_frame);
        };

        EventLoop::Ref().enqueueThreadPoolTrivialTask(async_executor, {});
    }

    ~VAAPIVBOAccessor() override {
        // `Release` will not be called if the frame is dropped.
        // In that case, frame will be freed here.
        FreeMappedFrame();
    }

    bool IsGpuBackedTexture(bool has_gpu_context) override {
        return has_gpu_context;
    }

    void Prefetch() override {}

    /**
     * How to use and composite a hardware (VAAPI) frame depends on whether the
     * Glamor context has a GPU context:
     *
     * - If Glamor context provides us with an active GPU context, the hardware
     *   frame will be copied into a Vulkan texture, then it will be wrapped
     *   into an GPU backed `SkImage` object.
     *
     * - If GPU context is not provided, frame data will be downloaded into
     *   CPU memory and a raster backed `SkImage` object will be returned.
     *
     * @note Actually, Vulkan allows us to import an external GPU texture directly
     *       without any copy. However, it requires `VK_EXT_image_drm_format_modifier`
     *       extension, which is not supported widely by all the GPU drivers.
     *       For example, in Mesa, it is not available for AMD GPUs under GFX8.
     */
    sk_sp<SkImage> Acquire(GrDirectContext *direct_context) override
    {
        TRACE_EVENT("multimedia", "VAAPIVBOAccessor::Acquire");

        map_frame_ = async_map_promise_->get_future().get();
        async_map_pending_ = false;
        if (!map_frame_)
            return nullptr;

        return create_skimage_from_frame(direct_context, map_frame_,
                                         scale_size_, sampling_,
                                         embedder_->GetSwsContextCache(), true);
    }

    void Release() override {
        FreeMappedFrame();
    }

    void FreeMappedFrame()
    {
        // `map_frame_` has been received by `Acquire` method
        if (map_frame_)
        {
            av_frame_free(&map_frame_);
            return;
        }

        // `map_frame_` has not been received by `Acquire` method yet
        // (`Acquire` is not called for some reasons like dropped frame)
        if (async_map_pending_)
        {
            map_frame_ = async_map_promise_->get_future().get();
            if (map_frame_)
                av_frame_free(&map_frame_);
            async_map_pending_ = false;
        }
    }

private:
    VideoFrameGLEmbedder *embedder_;
    AVFrame *map_frame_;
    SkISize scale_size_;
    SkSamplingOptions sampling_;
    bool async_map_pending_;
    std::shared_ptr<std::promise<AVFrame*>> async_map_promise_;
};

class HostVBOAccessor : public gl::ExternalTextureAccessor
{
public:
    HostVBOAccessor(VideoFrameGLEmbedder *embedder,
                    AVFrame *frame,
                    const SkISize& scale_size,
                    const SkSamplingOptions& sampling)
        : embedder_(embedder)
        , frame_(av_frame_clone(frame))
        , scale_size_(scale_size)
        , sampling_(sampling)
    {
    }

    ~HostVBOAccessor() override {
        if (frame_)
            av_frame_free(&frame_);
    }

    bool IsGpuBackedTexture(bool has_gpu_context) override {
        return has_gpu_context;
    }

    void Prefetch() override {}

    void Release() override {}

    sk_sp<SkImage> Acquire(GrDirectContext *direct) override
    {
        TRACE_EVENT("multimedia", "HostVBOAccessor::Acquire");

        return create_skimage_from_frame(direct, frame_,
                                         scale_size_, sampling_,
                                         embedder_->GetSwsContextCache(), false);
    }

private:
    VideoFrameGLEmbedder *embedder_;
    AVFrame *frame_;
    SkISize scale_size_;
    SkSamplingOptions sampling_;
};

} // namespace anonymous

VideoFrameGLEmbedder::VideoFrameGLEmbedder()
    : sws_context_cache_(std::make_unique<SwscaleContextCache>())
{
}

VideoFrameGLEmbedder::~VideoFrameGLEmbedder() = default;

std::unique_ptr<gl::ExternalTextureLayer>
VideoFrameGLEmbedder::Commit(const std::shared_ptr<VideoBuffer>& buffer,
                              const SkPoint& offset,
                              const SkISize& size,
                              const SkSamplingOptions& sampling)
{
    TRACE_EVENT("multimedia", "VideoFrameGLEmbedder::Commit");

    CHECK(buffer);

    auto *frame = buffer->CastUnderlyingPointer<AVFrame>();
    if (frame->linesize[0] < 0)
    {
        QLOG(LOG_ERROR, "Committing a vertical flipped frame (linesize < 0) is not supported");
        return nullptr;
    }

    if (size.width() <= 0 || size.height() <= 0)
    {
        QLOG(LOG_ERROR, "Invalid image dimensions ({}x{})", size.width(), size.height());
        return nullptr;
    }

    std::unique_ptr<gl::ExternalTextureAccessor> accessor;
    if (frame->format == AV_PIX_FMT_VAAPI)
    {
        // How to use and composite a hardware (VAAPI) frame depends on whether the
        // Glamor context has a GPU context. See `VAAPIVBOAccessor::Acquire` for more
        // details.
        accessor = std::make_unique<VAAPIVBOAccessor>(this, frame, size, sampling);
    }
    else
    {
        accessor = std::make_unique<HostVBOAccessor>(this, frame, size, sampling);
    }

    if (!accessor)
        return nullptr;

    return std::make_unique<gl::ExternalTextureLayer>(
            std::move(accessor), offset, size, sampling);
}

sk_sp<SkImage> VideoFrameGLEmbedder::ConvertToRasterImage(const std::shared_ptr<VideoBuffer>& buffer)
{
    TRACE_EVENT("multimedia", "VideoFrameGLEmbedder::ConvertToRasterImage");

    CHECK(buffer);

    auto *frame = buffer->CastUnderlyingPointer<AVFrame>();
    if (frame->linesize[0] < 0)
    {
        QLOG(LOG_ERROR, "Committing a vertical flipped frame (linesize < 0) is not supported");
        return nullptr;
    }

    SkISize size = SkISize::Make(frame->width, frame->height);

    std::unique_ptr<gl::ExternalTextureAccessor> accessor;
    if (frame->format == AV_PIX_FMT_VAAPI)
    {
        // How to use and composite a hardware (VAAPI) frame depends on whether the
        // Glamor context has a GPU context. See `VAAPIVBOAccessor::Acquire` for more
        // details.
        accessor = std::make_unique<VAAPIVBOAccessor>(
                this, frame, size, SkSamplingOptions(SkFilterMode::kLinear));
    }
    else
    {
        accessor = std::make_unique<HostVBOAccessor>(
                this, frame, size, SkSamplingOptions(SkFilterMode::kLinear));
    }

    if (!accessor)
        return nullptr;

    accessor->Prefetch();
    sk_sp<SkImage> image = accessor->Acquire(nullptr);
    accessor->Release();

    return image;
}

UTAU_NAMESPACE_END
