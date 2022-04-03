#ifndef COCOA_COBALT_RENDERCLIENTTRANSFER_H
#define COCOA_COBALT_RENDERCLIENTTRANSFER_H

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClient.h"
COBALT_NAMESPACE_BEGIN

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

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERCLIENTTRANSFER_H
