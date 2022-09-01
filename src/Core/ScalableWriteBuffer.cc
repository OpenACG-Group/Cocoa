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

#include <cstring>

#include "Core/Errors.h"
#include "Core/ScalableWriteBuffer.h"
#include "Core/Data.h"
namespace cocoa {

ScalableWriteBuffer::ScalableWriteBuffer(size_t cache_size)
    : buffer_start_ptr_(nullptr)
    , cache_start_ptr_(nullptr)
    , offset_in_cache_(0)
    , offset_in_buffer_(0)
    , cache_size_(cache_size)
{
    CHECK(cache_size_ > 0 && "Invalid size of cache memory");
    cache_start_ptr_ = reinterpret_cast<uint8_t*>(malloc(cache_size_));
    CHECK(cache_start_ptr_ && "Cache allocation failed");
}

ScalableWriteBuffer::~ScalableWriteBuffer()
{
    if (cache_start_ptr_)
        free(cache_start_ptr_);
    if (buffer_start_ptr_)
        free(buffer_start_ptr_);
}

void ScalableWriteBuffer::syncAndResetCacheContents()
{
    if (offset_in_cache_ == 0)
        return;

    if (!buffer_start_ptr_)
    {
        // Buffer has not been allocated yet.
        buffer_start_ptr_ = reinterpret_cast<uint8_t*>(malloc(offset_in_cache_));
        CHECK(buffer_start_ptr_ && "Buffer allocation failed");
    }
    else
    {
        // Buffer has been allocated, then reallocate it with
        // a proper size.
        size_t new_size = offset_in_buffer_ + offset_in_cache_;
        buffer_start_ptr_ = reinterpret_cast<uint8_t*>(realloc(buffer_start_ptr_, new_size));
        CHECK(buffer_start_ptr_ && "Buffer reallocation failed");
    }

    // Copy data and reset cache
    std::memcpy(buffer_start_ptr_ + offset_in_buffer_, cache_start_ptr_, cache_size_);
    offset_in_buffer_ += offset_in_cache_;
    offset_in_cache_ = 0;
}

void ScalableWriteBuffer::writeBytes(const uint8_t *src, size_t size)
{
    CHECK(cache_start_ptr_ && "Operating on finalized scalable buffer");

    while (size > 0)
    {
        size_t cache_remaining_size = cache_size_ - offset_in_cache_;

        size_t once_write_size = std::min(size, cache_remaining_size);
        std::memcpy(cache_start_ptr_ + offset_in_cache_, src, once_write_size);
        offset_in_cache_ += once_write_size;

        size -= once_write_size;
        src += once_write_size;

        if (offset_in_cache_ == cache_size_)
        {
            // This will make `offset_in_cache_ = 0`
            syncAndResetCacheContents();
        }
    }
}

void ScalableWriteBuffer::writeBytes(const std::shared_ptr<Data>& src, size_t size)
{
    CHECK(size <= src->size() && "Specified data size is out of range");
    CHECK(cache_start_ptr_ && "Operating on finalized scalable buffer");

    if (src->hasAccessibleBuffer())
    {
        writeBytes(reinterpret_cast<const uint8_t*>(src->getAccessibleBuffer()), size);
        return;
    }

    while (size > 0)
    {
        size_t cache_remaining_size = cache_size_ - offset_in_cache_;

        size_t once_write_size = std::min(size, cache_remaining_size);
        src->read(cache_start_ptr_ + offset_in_cache_, once_write_size);
        offset_in_cache_ += once_write_size;

        size -= once_write_size;

        if (offset_in_cache_ == cache_size_)
        {
            // This will make `offset_in_cache_ = 0`
            syncAndResetCacheContents();
        }
    }
}

std::shared_ptr<Data> ScalableWriteBuffer::finalize()
{
    CHECK(cache_start_ptr_ && "Operating on finalized scalable buffer");

    syncAndResetCacheContents();
    free(cache_start_ptr_);
    cache_start_ptr_ = nullptr;
    cache_size_ = 0;

    auto result = Data::MakeFromPtrWithoutCopy(buffer_start_ptr_, offset_in_buffer_, true);
    CHECK(result);

    buffer_start_ptr_ = nullptr;
    offset_in_buffer_ = 0;

    return result;
}

} // namespace cocoa
