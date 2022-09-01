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

#ifndef COCOA_GLAMOR_RENDERCLIENTTRANSFER_H
#define COCOA_GLAMOR_RENDERCLIENTTRANSFER_H

#include "Glamor/Glamor.h"
#include "Glamor/RenderClient.h"
GLAMOR_NAMESPACE_BEGIN

class RenderClientTransfer
{
public:
    enum class Type
    {
        kInvocationResponse,
        kSignalEmit
    };

    using Timepoint = std::chrono::steady_clock::time_point;

    explicit RenderClientTransfer(Type type) : type_(type) {}
    virtual ~RenderClientTransfer() = default;

    g_nodiscard g_inline bool IsInvocationResponse() const {
        return (type_ == Type::kInvocationResponse);
    }

    g_nodiscard g_inline bool IsSignalEmit() const {
        return (type_ == Type::kSignalEmit);
    }

    g_inline void MarkProfileMilestone(ITCProfileMilestone tag) {
        profile_milestones_[static_cast<uint8_t>(tag)] = std::chrono::steady_clock::now();
    }

    g_nodiscard g_inline Timepoint GetProfileMilestone(ITCProfileMilestone tag) const {
        return profile_milestones_[static_cast<uint8_t>(tag)];
    }

private:
    static constexpr size_t kMilestonesSize = static_cast<uint8_t>(ITCProfileMilestone::kLast) + 1;

    Type        type_;
    Timepoint   profile_milestones_[kMilestonesSize];
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERCLIENTTRANSFER_H
