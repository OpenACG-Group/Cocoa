#ifndef COCOA_GLAMOR_MOE_MOEEXTERNALBREAKPOINTHANDLER_H
#define COCOA_GLAMOR_MOE_MOEEXTERNALBREAKPOINTHANDLER_H

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class MoeHeap;

class MoeExternalBreakpointHandler
{
public:
    enum class Result
    {
        kContinue,
        kInterrupt,
        kRaiseException
    };

    using BreakpointID = uint32_t;

    virtual ~MoeExternalBreakpointHandler() = default;

    virtual Result OnDebugBreakpoint(BreakpointID id, MoeHeap& heap) noexcept = 0;
    virtual Result OnRelocationBreakpoint(BreakpointID id) noexcept = 0;
    virtual Result OnProfilingBreakpoint(BreakpointID id) noexcept = 0;
};


GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOE_MOEEXTERNALBREAKPOINTHANDLER_H
