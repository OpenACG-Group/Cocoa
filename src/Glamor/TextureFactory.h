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

#ifndef COCOA_GLAMOR_TEXTUREFACTORY_H
#define COCOA_GLAMOR_TEXTUREFACTORY_H

#include <utility>

#include "include/core/SkPixmap.h"
#include "include/core/SkImage.h"

#include "Core/Data.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class Texture;
class HWComposeSwapchain;

class TextureFactory
{
public:
    TextureFactory() = default;
    virtual ~TextureFactory() = default;

    Shared<Texture> MakeFromEncodedData(const Shared<Data>& data,
                                        std::optional<SkAlphaType> alpha_type = std::nullopt);

    Shared<Texture> MakeFromEncodedData(const sk_sp<SkData>& data,
                                        std::optional<SkAlphaType> alpha_type = std::nullopt);

    Shared<Texture> MakeFromPixmap(const SkPixmap& pixmap);

    Shared<Texture> MakeFromImage(const sk_sp<SkImage>& image);

    // TODO(sora): add supporting of DMABUF textures (wayland screencast)
    // TODO(sora): add supporting of custom textures (user's extensions)

protected:
    virtual Shared<Texture> OnMakeFromRawData(const void *pixels, const SkImageInfo& info) = 0;
    virtual Shared<Texture> OnMakeFromImage(const sk_sp<SkImage>& image) = 0;
};

class RasterTextureFactory : public TextureFactory
{
public:
    explicit RasterTextureFactory(SkColorInfo color_info)
        : color_info_(std::move(color_info)) {}
    ~RasterTextureFactory() override = default;

    Shared<Texture> OnMakeFromImage(const sk_sp<SkImage> &image) override;
    Shared<Texture> OnMakeFromRawData(const void *pixels, const SkImageInfo &info) override;

private:
    SkColorInfo     color_info_;
};

class HWComposeTextureFactory : public TextureFactory
{
public:
    explicit HWComposeTextureFactory(Shared<HWComposeSwapchain> swapchain)
        : swapchain_(std::move(swapchain)) {}
    ~HWComposeTextureFactory() override = default;

    Shared<Texture> OnMakeFromRawData(const void *pixels, const SkImageInfo &info) override;
    Shared<Texture> OnMakeFromImage(const sk_sp<SkImage> &image) override;

private:
    Shared<HWComposeSwapchain>  swapchain_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_TEXTUREFACTORY_H
