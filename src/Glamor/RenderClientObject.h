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

#ifndef COCOA_GLAMOR_RENDERCLIENTOBJECT_H
#define COCOA_GLAMOR_RENDERCLIENTOBJECT_H

#include <map>

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderClientCallInfo.h"
#include "Glamor/RenderClientEmitterInfo.h"
#include "Glamor/RenderClientSignalEmit.h"
#include "Glamor/RenderHostSlotCallbackInfo.h"
#include "Glamor/RenderHostCallbackInfo.h"
GLAMOR_NAMESPACE_BEGIN

using RenderClientCallTrampoline = void(*)(RenderClientCallInfo&);

#define GLAMOR_TRAMPOLINE_IMPL(class_, method) \
void class_##_##method##_Trampoline(RenderClientCallInfo& info)

#define GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(n)                                  \
    do {                                                                        \
        if (info.Length() != (n)) {                                             \
            info.SetReturnStatus(RenderClientCallInfo::Status::kArgsInvalid);   \
            return;                                                             \
        }                                                                       \
    } while (false)

class RenderClientObject : public std::enable_shared_from_this<RenderClientObject>
{
    friend class RenderClient;

public:
    enum class RealType
    {
        kRenderHostTaskRunner,
        kRenderHostCreator,
        kDisplay,
        kSurface,
        kBlender,
        kMonitor,
        kCursorTheme,
        kCursor
    };

    using OpCode = RenderClientCallInfo::OpCode;
    using ReturnStatus = RenderClientCallInfo::Status;
    using SignalCode = RenderClientSignalEmit::SignalCode;

    explicit RenderClientObject(RealType type);
    virtual ~RenderClientObject();

    /**
     * Get a string representation of `RealType` enumeration.
     * Crash the program if `type` is an unexpected enumeration value;
     * always return a valid string pointer.
     */
    g_nodiscard static const char *GetTypeName(RealType type);

    template<typename T>
    g_nodiscard g_inline Shared<T> Cast() {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    g_nodiscard g_inline Shared<RenderClientObject> Self() {
        return shared_from_this();
    }

    template<typename T, typename std::enable_if<std::is_base_of_v<RenderClientObject, T>>::type* = nullptr>
    g_nodiscard g_inline Shared<T> As() {
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

    uint32_t Connect(SignalCode signal, const RenderHostSlotCallback& callback, bool localThread = false);
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
    g_private_api void EmitterTrampoline(const std::shared_ptr<RenderClientSignalEmit>& emit, bool localThread);

private:
    static constexpr OpCode kTrampolinePoolInitSize = 32;

    struct ConnectedSlot
    {
        uint32_t id;
        RenderHostSlotCallback callback;
        bool local_thread;
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

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_RENDERCLIENTOBJECT_H
