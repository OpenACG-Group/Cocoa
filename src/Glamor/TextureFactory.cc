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

#include "include/core/SkData.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

#include "Core/Errors.h"
#include "Core/Journal.h"
#include "Glamor/TextureFactory.h"
#include "Glamor/Texture.h"
#include "Glamor/HWComposeSwapchain.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.TextureFactory)

// `Texture` implementation

Shared<Texture> TextureFactory::MakeFromEncodedData(const Shared<Data>& data,
                                                    std::optional<SkAlphaType> alpha_type)
{
    CHECK(data);

    Shared<Data> linear_buffer = data;
    if (!data->hasAccessibleBuffer())
    {
        linear_buffer = Data::MakeLinearBuffer(data);
        if (!linear_buffer)
            return nullptr;
    }

    return MakeFromEncodedData(SkData::MakeWithoutCopy(linear_buffer->getAccessibleBuffer(),
                                                       linear_buffer->size()),
                               alpha_type);
}

Shared<Texture> TextureFactory::MakeFromEncodedData(const sk_sp<SkData>& data,
                                                    std::optional<SkAlphaType> alpha_type)
{
    CHECK(data);

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
    if (!codec)
        return nullptr;

    auto [image, result] = codec->getImage();
    if (result != SkCodec::Result::kSuccess)
    {
        QLOG(LOG_ERROR, "Failed to decode image: {}", SkCodec::ResultToString(result));
        return nullptr;
    }

    return MakeFromImage(image);
}

Shared<Texture> TextureFactory::MakeFromImage(const sk_sp<SkImage>& image)
{
    CHECK(image);
    return this->OnMakeFromImage(image);
}

Shared<Texture> TextureFactory::MakeFromPixmap(const SkPixmap& pixmap)
{
    if (pixmap.width() == 0 || pixmap.height() == 0)
        return nullptr;
    if (!pixmap.addr())
        return nullptr;

    return this->OnMakeFromRawData(pixmap.addr(), pixmap.info());
}

// `HWComposeTextureFactory` implementation

namespace {
// This global variable will only be accessed by rendering thread,
// so no mutex lock or thread_local attribute are applied.
Texture::TextureId g_texture_id_counter = 0;
} // namespace anonymous

Shared<Texture> RasterTextureFactory::OnMakeFromImage(const sk_sp<SkImage>& image)
{
    sk_sp<SkImage> source_image = image;
    if (image->isTextureBacked())
    {
        source_image = image->makeRasterImage();
        if (!source_image)
        {
            QLOG(LOG_ERROR, "Failed to create a raster texture from a GPU image");
            return nullptr;
        }
    }

    // Now `source_image` must be a raster-backed image

    sk_sp<SkImage> texture_image;

    SkColorType color_type = color_info_.colorType();
    SkAlphaType alpha_type = color_info_.alphaType();
    if (source_image->imageInfo().colorType() == color_type &&
        source_image->imageInfo().alphaType() == alpha_type)
    {
        // No color conversion should be performed.
        // `makeSubset` clones the source image and copies pixels into a new image
        texture_image = source_image->makeSubset(source_image->bounds());
    }
    else
    {
        auto image_info = SkImageInfo::Make(source_image->imageInfo().dimensions(),
                                            color_type,
                                            alpha_type,
                                            SkColorSpace::MakeSRGB());

        SkPixmap pixels;
        if (!source_image->peekPixels(&pixels))
        {
            QLOG(LOG_ERROR, "Failed to peek pixels in the source image");
            return nullptr;
        }

        SkBitmap dst;
        dst.allocPixels(image_info, image_info.minRowBytes());
        // `writePixels` performs color conversion
        dst.writePixels(pixels);

        dst.setImmutable();
        // `setImmutable` makes `asImage` returns an image object which shares the pixels
        // with the bitmap, which means no pixels duplication is performed.
        texture_image = dst.asImage();
    }

    return std::make_shared<Texture>(++g_texture_id_counter,
                                     texture_image->imageInfo(),
                                     false,
                                     texture_image);
}

Shared<Texture> RasterTextureFactory::OnMakeFromRawData(const void *pixels, const SkImageInfo& info)
{
    CHECK(pixels);

    SkPixmap pixmap(info, pixels, info.minRowBytes());
    return OnMakeFromImage(SkImages::RasterFromPixmapCopy(pixmap));
}

Shared<Texture> HWComposeTextureFactory::OnMakeFromImage(const sk_sp<SkImage>& image)
{
    GrDirectContext *context = swapchain_->GetSkiaDirectContext().get();
    CHECK(context);

    SkColorType target_color_type = swapchain_->GetImageFormat();
    SkAlphaType target_alpha_type = swapchain_->GetImageAlphaFormat();

    sk_sp<SkImage> texture_image;
    if (image->imageInfo().colorType() == target_color_type &&
        image->imageInfo().alphaType() == target_alpha_type)
    {
        // No color conversion should be performed.
        // `makeSubset` makes the image turn to a GPU-backend texture.
        texture_image = image->makeSubset(image->bounds(), context);
    }
    else
    {
        // `SkImage::makeColorTypeAndColorSpace` is still an experimental API
        // in Skia, so we convert color format with the help of `SkSurface`.

        auto image_info = SkImageInfo::Make(image->imageInfo().dimensions(),
                                            target_color_type,
                                            target_alpha_type,
                                            SkColorSpace::MakeSRGB());

        sk_sp<SkSurface> surface = SkSurfaces::RenderTarget(context, skgpu::Budgeted::kNo, image_info);
        if (!surface)
        {
            // `SkSurfaces::RenderTarget` fails for many reasons. In most cases
            // it is because the parameters are invalid.
            return nullptr;
        }
        surface->getCanvas()->drawImage(image, 0, 0);

        texture_image = surface->makeImageSnapshot();
        if (!texture_image)
            return nullptr;
    }

    return std::make_shared<Texture>(++g_texture_id_counter,
                                     texture_image->imageInfo(),
                                     true,
                                     texture_image);
}

Shared<Texture> HWComposeTextureFactory::OnMakeFromRawData(const void *pixels,
                                                           const SkImageInfo& info)
{
    CHECK(pixels);

    SkPixmap pixmap(info, pixels, info.minRowBytes());
    return OnMakeFromImage(SkImages::RasterFromPixmapCopy(pixmap));
}

GLAMOR_NAMESPACE_END
