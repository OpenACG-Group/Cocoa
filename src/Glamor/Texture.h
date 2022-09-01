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


#ifndef COCOA_GLAMOR_TEXTURE_H
#define COCOA_GLAMOR_TEXTURE_H

#include <utility>

#include "include/core/SkImageInfo.h"
#include "include/core/SkImage.h"

#include "Glamor/Glamor.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

/**
 * `Texture` represents an image which is stored in the GPU or CPU memory.
 * It only can be accessed directly from the rendering thread, and other
 * threads should access it with a unique ID by `TextureManager` interface.
 */
class Texture : public GraphicsResourcesTrackable
{
public:
    using TextureId = int64_t;

    Texture(TextureId id,
            SkImageInfo image_info,
            bool is_hwcompose_texture,
            const sk_sp<SkImage>& texture_image);
    ~Texture() override = default;

    g_nodiscard g_inline TextureId GetUniqueId() const {
        return unique_id_;
    }

    g_nodiscard g_inline const SkImageInfo& GetImageInfo() const {
        return image_info_;
    }

    g_nodiscard g_inline const sk_sp<SkImage>& GetImage() const {
        return texture_image_;
    }

    g_nodiscard g_inline bool IsHWComposeTexture() const {
        return is_hwcompose_texture_;
    }

    void Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept override;

private:
    TextureId               unique_id_;
    SkImageInfo             image_info_;
    bool                    is_hwcompose_texture_;
    sk_sp<SkImage>          texture_image_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_TEXTURE_H
