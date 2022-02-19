#ifndef COCOA_COBALT_RENDERCLIENT_H
#define COCOA_COBALT_RENDERCLIENT_H

#include <map>

#include "Core/Errors.h"
#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientCallInfo.h"
#include "Cobalt/RenderClientEmitterInfo.h"
#include "Cobalt/RenderClientSignalEmit.h"
#include "Cobalt/RenderHostSlotCallbackInfo.h"
#include "Cobalt/RenderHostCallbackInfo.h"
COBALT_NAMESPACE_BEGIN

using RenderClientCallTrampoline = void(*)(RenderClientCallInfo&);

#define COBALT_TRAMPOLINE_IMPL(class_, method) \
void class_##_##method##_Trampoline(RenderClientCallInfo& info)

class RenderClientObject : public std::enable_shared_from_this<RenderClientObject>
{
public:
    enum class RealType
    {
        kRenderHostCreator,
        kDisplay
    };

    using OpCode = RenderClientCallInfo::OpCode;
    using ReturnStatus = RenderClientCallInfo::Status;
    using SignalCode = RenderClientSignalEmit::SignalCode;

    explicit RenderClientObject(RealType type);
    virtual ~RenderClientObject();

    g_nodiscard g_inline co_sp<RenderClientObject> Self() {
        return shared_from_this();
    }

    template<typename T, typename std::enable_if<std::is_base_of_v<RenderClientObject, T>>::type* = nullptr>
    g_nodiscard g_inline co_sp<T> As() {
        auto ptr = std::dynamic_pointer_cast<T>(shared_from_this());
        CHECK(ptr && "Bad cast from RenderClient");
        return ptr;
    }

    g_nodiscard g_inline RealType GetRealType() const {
        return real_type_;
    }

    template<typename T, typename...ArgsT>
    void Invoke(OpCode opcode, T&& closure, const RenderHostCallback& callback, ArgsT&&...args) {
        RenderClientCallInfo info(opcode);
        info.SetClosure(std::forward<T>(closure));
        (info.SwallowBack(std::any(std::forward<ArgsT>(args))), ...);
        Invoke(std::move(info), callback);
    }

    void Invoke(RenderClientCallInfo info, const RenderHostCallback& callback);

    uint32_t Connect(SignalCode signal, const RenderHostSlotCallback& callback);
    void Disconnect(uint32_t id);

    g_nodiscard g_inline const RenderHostCallback& DummyHostCallback() const {
        return dummy_host_callback_;
    }

    g_nodiscard g_inline uint32_t GetDanglingCallbacksCounter() const {
        return dangling_callbacks_counter_;
    }

    g_private_api void Emit(SignalCode signal, RenderClientEmitterInfo info);

    g_private_api void CallFromHostTrampoline(RenderClientCallInfo& info);
    g_private_api void SetMethodTrampoline(OpCode opcode, RenderClientCallTrampoline func);
    g_private_api void EmitterTrampoline(RenderClientSignalEmit *emit);

private:
    static constexpr OpCode kTrampolinePoolInitSize = 32;

    struct ConnectedSlot
    {
        uint32_t id;
        RenderHostSlotCallback callback;
    };

    RealType                    real_type_;
    RenderClientCallTrampoline *trampolines_pool_;
    OpCode                      trampolines_pool_size_;
    uint32_t                    slot_id_counter_;
    std::multimap<SignalCode, ConnectedSlot> signal_slots_map_;
    std::mutex                  signal_slots_map_lock_;
    RenderHostCallback          dummy_host_callback_;
    uint32_t                    dangling_callbacks_counter_;
};

COBALT_NAMESPACE_END
#endif //COCOA_COBALT_RENDERCLIENT_H
