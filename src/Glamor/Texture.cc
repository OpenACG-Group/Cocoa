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

#include "Glamor/Texture.h"
GLAMOR_NAMESPACE_BEGIN

Texture::Texture(TextureId id,
                 SkImageInfo image_info,
                 bool is_hwcompose_texture,
                 const sk_sp<SkImage>& texture_image)
    : unique_id_(id)
    , image_info_(std::move(image_info))
    , is_hwcompose_texture_(is_hwcompose_texture)
    , texture_image_(texture_image)
{
}

void Texture::Trace(GraphicsResourcesTrackable::Tracer *tracer) noexcept
{
    size_t texture_size;
    if (texture_image_->isTextureBacked())
        texture_size = texture_image_->textureSize();
    else
        texture_size = texture_image_->imageInfo().computeMinByteSize();

    tracer->TraceResource("SkImage",
                          TRACKABLE_TYPE_TEXTURE,
                          is_hwcompose_texture_ ? TRACKABLE_DEVICE_GPU
                                                : TRACKABLE_DEVICE_CPU,
                          TRACKABLE_OWNERSHIP_STRICT_OWNED,
                          TraceIdFromPointer(texture_image_.get()),
                          texture_size);
}

GLAMOR_NAMESPACE_END
