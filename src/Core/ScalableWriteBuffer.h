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

#ifndef COCOA_CORE_SCALABLEWRITEBUFFER_H
#define COCOA_CORE_SCALABLEWRITEBUFFER_H

#include <memory>

#include "Core/Project.h"
namespace cocoa {

class Data;

/**
 * A memory buffer which can grow in size.
 * The size of buffer cannot be changed any more after calling `finalize` method,
 * and the size will equal to the number of bytes written in buffer accurately.
 */
class ScalableWriteBuffer
{
public:
    constexpr static size_t kDefaultCacheSize = 1024;

    /**
     * Construct a scalable buffer with a specified cache size.
     * Data will be written into cache initially, then they will be copied into
     * actual buffer when the cache space is full. If the actual is not enough to
     * receive the contents in the cache, it will be reallocated with a proper size.
     */
    explicit ScalableWriteBuffer(size_t cache_size = kDefaultCacheSize);
    ~ScalableWriteBuffer();

    void writeBytes(const uint8_t *src, size_t size);
    void writeBytes(const std::shared_ptr<Data>& src, size_t size);

    std::shared_ptr<Data> finalize();

private:
    void syncAndResetCacheContents();

    uint8_t                 *buffer_start_ptr_;
    uint8_t                 *cache_start_ptr_;
    uint64_t                 offset_in_cache_;
    uint64_t                 offset_in_buffer_;
    size_t                   cache_size_;
};

} // namespace cocoa
#endif //COCOA_CORE_SCALABLEWRITEBUFFER_H
