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

#ifndef COCOA_GLAMOR_PRESENTSIGNALARGS_H
#define COCOA_GLAMOR_PRESENTSIGNALARGS_H

#include <functional>
#include <any>
#include <vector>

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class PresentSignal;
class PresentRemoteHandle;

class PresentSignalArgs
{
public:
    explicit PresentSignalArgs(PresentSignal& signal_info);
    PresentSignalArgs(const PresentSignalArgs&) = delete;
    ~PresentSignalArgs() = default;

    template<typename T>
    g_nodiscard g_inline T& Get(size_t index) {
        CHECK(index < args_vector_ref_.size());
        return std::any_cast<T&>(args_vector_ref_[index]);
    }

    g_nodiscard g_inline std::any& Get(size_t index) {
        CHECK(index < args_vector_ref_.size());
        return args_vector_ref_[index];
    }

    g_nodiscard g_inline size_t Length() const {
        return args_vector_ref_.size();
    }

private:
    PresentSignal&           signal_info_;
    std::vector<std::any>&   args_vector_ref_;
};

using PresentSignalCallback = std::function<void(PresentSignalArgs&)>;

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTSIGNALARGS_H
