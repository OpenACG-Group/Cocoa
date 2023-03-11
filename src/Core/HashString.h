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

#ifndef COCOA_CORE_HASHSTRING_H
#define COCOA_CORE_HASHSTRING_H

#include <cstdint>

#include "Core/Project.h"
COCOA_BEGIN_NAMESPACE

template<typename Base>
class HashString
{
public:
    explicit HashString(const Base& base)
        : hash_(std::hash<Base>()(base))
        , base_(base) {}

    explicit HashString(Base&& base)
        : hash_(std::hash<Base>()(base))
        , base_(base) {}

    HashString(const HashString<Base>& other) = default;
    HashString(HashString<Base>&& other) noexcept
        : hash_(other.hash_)
        , base_(std::move(other.base_)) {
        other.hash_ = 0;
    }

    g_nodiscard g_inline const Base& Get() const {
        return base_;
    }

    g_nodiscard g_inline uint64_t GetHash() const {
        return hash_;
    }

    template<typename T>
    bool operator==(const HashString<T>& other) const {
        if (other.GetHash() != this->hash_)
            return false;
        return (other.Get() == this->base_);
    }

private:
    uint64_t    hash_;
    Base        base_;
};

COCOA_END_NAMESPACE
#endif //COCOA_CORE_HASHSTRING_H
