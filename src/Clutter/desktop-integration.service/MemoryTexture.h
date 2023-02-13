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

#ifndef COCOA_GLAMOR_WAYLAND_SCREENCAST_MEMORYTEXTURE_H
#define COCOA_GLAMOR_WAYLAND_SCREENCAST_MEMORYTEXTURE_H

#include <functional>

#include "Screencast.h"
#include "Errors.h"
SCREENCAST_NAMESPACE_BEGIN

class TextureInfo
{
public:
    constexpr static int32_t kU32BytesPerPixel = 4;

    TextureInfo(TextureFormat format, int32_t width, int32_t height);
    ~TextureInfo() = default;

    g_nodiscard g_inline bool operator==(const TextureInfo& other) const {
        return (other.format_ == format_ &&
                other.width_ == width_ &&
                other.height_ == height_);
    }

    g_nodiscard g_inline bool operator!=(const TextureInfo& other) const {
        return (other.format_ != format_ ||
                other.width_ != width_ ||
                other.height_ != height_);
    }

    g_nodiscard g_inline TextureFormat GetFormat() const {
        return format_;
    }

    g_nodiscard g_inline int32_t GetWidth() const {
        return width_;
    }

    g_nodiscard g_inline int32_t GetHeight() const {
        return height_;
    }

    g_nodiscard int32_t GetBytesPerPixel() const;

    g_nodiscard size_t ComputeMinByteSize() const;

    g_nodiscard size_t ComputeMinStride() const;

private:
    TextureFormat   format_;
    int32_t         width_;
    int32_t         height_;
};

class MemoryTexture
{
public:
    using StorageReleaser = std::function<void(uint8_t*)>;

    MemoryTexture(const TextureInfo& info, uint8_t *ptr,
                  StorageReleaser releaser);
    ~MemoryTexture();

    g_nodiscard static std::unique_ptr<MemoryTexture> Allocate(const TextureInfo& info);

    g_nodiscard g_inline const TextureInfo& GetInfo() const {
        return texture_info_;
    }

    g_nodiscard g_inline uint8_t *GetStorage() const {
        CHECK(texture_storage_);
        return texture_storage_;
    }

    g_nodiscard size_t SerializeWithInfo(uint8_t *dst, size_t dst_capacity);

private:
    TextureInfo         texture_info_;
    uint8_t            *texture_storage_;
    StorageReleaser     storage_releaser_;
};

SCREENCAST_NAMESPACE_END
#endif //COCOA_GLAMOR_WAYLAND_SCREENCAST_MEMORYTEXTURE_H
