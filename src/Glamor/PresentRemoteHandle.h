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

#ifndef COCOA_GLAMOR_PRESENTREMOTEHANDLE_H
#define COCOA_GLAMOR_PRESENTREMOTEHANDLE_H

#include <map>

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteCall.h"
#include "Glamor/PresentSignal.h"
#include "Glamor/PresentSignalMessage.h"
#include "Glamor/PresentSignalArgs.h"
#include "Glamor/PresentRemoteCallReturn.h"
GLAMOR_NAMESPACE_BEGIN

#define GLAMOR_TRAMPOLINE_IMPL(class_, method) \
void class_##_##method##_Trampoline(PresentRemoteCall& info)

#define GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(n)                                  \
    do {                                                                        \
        if (info.Length() != (n)) {                                             \
            info.SetReturnStatus(PresentRemoteCall::Status::kArgsInvalid);   \
            return;                                                             \
        }                                                                       \
    } while (false)

class PresentRemoteHandle : public std::enable_shared_from_this<PresentRemoteHandle>
{
public:
    enum class RealType
    {
        kTaskRunner,
        kDisplay,
        kSurface,
        kBlender,
        kMonitor,
        kCursorTheme,
        kCursor
    };

    using OpCode = PresentRemoteCall::OpCode;
    using ReturnStatus = PresentRemoteCall::Status;
    using SignalCode = PresentSignalMessage::SignalCode;

    using RemoteCallTrampoline = void(*)(PresentRemoteCall&);

    explicit PresentRemoteHandle(RealType type);
    virtual ~PresentRemoteHandle();

    /**
     * Get a string representation of `RealType` enumeration.
     * Crash the program if `type` is an unexpected enumeration value;
     * always return a valid string pointer.
     */
    g_nodiscard static const char *GetTypeName(RealType type);

    template<typename T>
    g_nodiscard g_inline Shared<T> Cast() {
        return std::static_pointer_cast<T>(shared_from_this());
    }

    g_nodiscard g_inline Shared<PresentRemoteHandle> Self() {
        return shared_from_this();
    }

    template<typename T, typename std::enable_if<std::is_base_of_v<PresentRemoteHandle, T>>::type* = nullptr>
    g_nodiscard g_inline Shared<T> As() {
        auto ptr = std::static_pointer_cast<T>(shared_from_this());
        return ptr;
    }

    g_nodiscard g_inline RealType GetRealType() const {
        return real_type_;
    }

    template<typename T, typename...ArgsT>
    void Invoke(OpCode opcode, T&& closure, const PresentRemoteCallResultCallback& callback, ArgsT&&...args) {
        PresentRemoteCall info(opcode);
        info.SetClosure(std::forward<T>(closure));
        (info.SwallowBack(std::any(std::forward<ArgsT>(args))), ...);
        Invoke(std::move(info), callback);
    }

    void Invoke(PresentRemoteCall info, const PresentRemoteCallResultCallback& callback);

    uint32_t Connect(SignalCode signal, const PresentSignalCallback& callback, bool localThread = false);
    void Disconnect(uint32_t id);

    g_nodiscard g_inline const PresentRemoteCallResultCallback& DummyHostCallback() const {
        return dummy_host_callback_;
    }

    g_nodiscard g_inline uint32_t GetDanglingCallbacksCounter() const {
        return dangling_callbacks_counter_;
    }

    g_private_api void Emit(SignalCode signal, PresentSignal info);

    g_private_api void DoRemoteCall(PresentRemoteCall& info);
    g_private_api void DoEmitSignal(SignalCode signal_code, PresentSignal& info, bool local_thread);

    g_private_api void SetMethodTrampoline(OpCode opcode, RemoteCallTrampoline func);

private:
    static constexpr OpCode kTrampolinePoolInitSize = 32;

    struct ConnectedSlot
    {
        uint32_t id;
        PresentSignalCallback callback;
        bool local_thread;
    };

    RealType                    real_type_;
    RemoteCallTrampoline       *trampolines_pool_;
    OpCode                      trampolines_pool_size_;
    uint32_t                    slot_id_counter_;
    std::multimap<SignalCode, ConnectedSlot>
                                signal_slots_map_;
    std::mutex                  signal_slots_map_lock_;
    PresentRemoteCallResultCallback          dummy_host_callback_;
    uint32_t                    dangling_callbacks_counter_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_PRESENTREMOTEHANDLE_H
