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

#include <cstdlib>
#include <utility>

#include "MemoryTexture.h"
SCREENCAST_NAMESPACE_BEGIN

TextureInfo::TextureInfo(TextureFormat format, int32_t width, int32_t height)
    : format_(format)
    , width_(width)
    , height_(height)
{
    CHECK(format != TextureFormat::kUnknown);
    CHECK(width > 0 && height > 0);
}

int32_t TextureInfo::GetBytesPerPixel() const
{
    CHECK(format_ != TextureFormat::kUnknown);
    return kU32BytesPerPixel;
}

size_t TextureInfo::ComputeMinStride() const
{
    return (width_ * GetBytesPerPixel());
}

size_t TextureInfo::ComputeMinByteSize() const
{
    return (width_ * height_ * GetBytesPerPixel());
}

MemoryTexture::MemoryTexture(const TextureInfo& info, uint8_t *ptr,
                             StorageReleaser releaser)
    : texture_info_(info)
    , texture_storage_(ptr)
    , storage_releaser_(std::move(releaser))
{
    CHECK(texture_storage_);
    CHECK(storage_releaser_);
}

MemoryTexture::~MemoryTexture()
{
    CHECK(storage_releaser_ && texture_storage_);
    storage_releaser_(texture_storage_);
}

std::unique_ptr<MemoryTexture> MemoryTexture::Allocate(const TextureInfo& info)
{
    auto *storage = reinterpret_cast<uint8_t *>(malloc(info.ComputeMinByteSize()));
    CHECK(storage);

    return std::make_unique<MemoryTexture>(info, storage, [](uint8_t *ptr) {
        free(ptr);
    });
}

size_t MemoryTexture::SerializeWithInfo(uint8_t *dst, size_t dst_capacity)
{
    // TODO: implement this.
    return 0;
}

SCREENCAST_NAMESPACE_END
