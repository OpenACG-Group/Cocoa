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

#ifndef COCOA_GLAMOR_PRESENTSIGNAL_H
#define COCOA_GLAMOR_PRESENTSIGNAL_H

#include <vector>
#include <any>

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class PresentRemoteHandle;

class PresentSignal
{
public:
    PresentSignal() = default;
    PresentSignal(const PresentSignal&) = delete;
    PresentSignal(PresentSignal&& rhs) noexcept
        : args_vector_(std::move(rhs.args_vector_)) {}
    ~PresentSignal() = default;

    template<typename T, typename...Args>
    PresentSignal& EmplaceBack(Args&&...args) {
        args_vector_.emplace_back(std::in_place_type_t<T>{}, std::forward<Args>(args)...);
        return *this;
    }

    template<typename T>
    PresentSignal& PushBack(T&& value) {
        args_vector_.push_back(value);
        return *this;
    }

    g_nodiscard size_t Length() const {
        return args_vector_.size();
    }

    g_private_api g_nodiscard std::vector<std::any>& GetArgsVector() {
        return args_vector_;
    }

private:
    std::vector<std::any>       args_vector_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTSIGNAL_H
