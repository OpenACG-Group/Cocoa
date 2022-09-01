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

#ifndef COCOA_GALLIUM_STACKALLOCATORBASE_H
#define COCOA_GALLIUM_STACKALLOCATORBASE_H

#include "Core/Errors.h"
#include <vector>

#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

template<std::size_t kStackAllocSize = 1024>
class StackAllocatorBase
{
public:
    StackAllocatorBase()
        : stack_{0}
        , stack_pos_(0) {}
    /* Noncopyable and unmovable */
    StackAllocatorBase(const StackAllocatorBase&) = delete;
    void *operator new(std::size_t) = delete;
    void operator delete(void*) = delete;

    ~StackAllocatorBase() = default;

    g_nodiscard std::size_t size() const {
        return stack_pos_;
    }

    uint8_t *increaseStackAlloc(std::size_t length) {
        if (stack_pos_ + length <= kStackAllocSize)
        {
            stack_pos_ += length;
            return stack_ + (stack_pos_ - length);
        }
        throw std::range_error("StackAllocatorBase: Stack space not enough");
    }

    void reduceStackAlloc(std::size_t length) {
        if (length >= stack_pos_)
            stack_pos_ = 0;
        else
            stack_pos_ -= length;
    }

private:
    uint8_t         stack_[kStackAllocSize];
    std::size_t     stack_pos_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_STACKALLOCATORBASE_H
