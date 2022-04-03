#ifndef COCOA_COBALT_RENDERCLIENTSIGNALEMIT_H
#define COCOA_COBALT_RENDERCLIENTSIGNALEMIT_H

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientTransfer.h"
#include "Cobalt/RenderClientEmitterInfo.h"
COBALT_NAMESPACE_BEGIN

class RenderClientSignalEmit : public RenderClientTransfer
{
public:
    using SignalCode = uint32_t;

    RenderClientSignalEmit(RenderClientEmitterInfo info, const co_sp<RenderClientObject>& emitter,
                           SignalCode code)
        : RenderClientTransfer(RenderClientTransfer::Type::kSignalEmit)
        , emitter_(emitter)
        , signal_code_(code)
        , args_vector_(info.MoveArgs()) {}
    ~RenderClientSignalEmit() override = default;

    g_nodiscard g_inline co_sp<RenderClientObject> GetEmitter() const {
        return emitter_;
    }

    g_nodiscard g_inline std::vector<std::any>& GetArgs() {
        return args_vector_;
    }

    g_nodiscard g_inline SignalCode GetSignalCode() const {
        return signal_code_;
    }

private:
    co_sp<RenderClientObject>   emitter_;
    SignalCode                  signal_code_;
    std::vector<std::any>       args_vector_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERCLIENTSIGNALEMIT_H
