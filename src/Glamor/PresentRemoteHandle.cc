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

#include "Core/Journal.h"

#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteHandle.h"
#include "Glamor/PresentSignalMessage.h"
#include "Glamor/PresentThread.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.PresentRemoteHandle)

PresentRemoteHandle::PresentRemoteHandle(RealType type)
    : real_type_(type)
    , trampolines_pool_(nullptr)
    , trampolines_pool_size_(kTrampolinePoolInitSize)
    , slot_id_counter_(0)
    , dangling_callbacks_counter_(0)
{
    trampolines_pool_ = reinterpret_cast<RemoteCallTrampoline*>(
            calloc(sizeof(OpCode), kTrampolinePoolInitSize));
    CHECK(trampolines_pool_);

    dummy_host_callback_ = [this](PresentRemoteCallReturn& info) {
        this->dangling_callbacks_counter_++;
    };
}

PresentRemoteHandle::~PresentRemoteHandle()
{
    if (trampolines_pool_)
        free(trampolines_pool_);
}

void PresentRemoteHandle::DoRemoteCall(PresentRemoteCall& info)
{
    info.SetThis(shared_from_this());
    if (info.GetOpCode() >= trampolines_pool_size_ || !trampolines_pool_[info.GetOpCode()])
    {
        info.SetReturnStatus(PresentRemoteCall::Status::kOpCodeInvalid);
    }
    else
    {
        try
        {
            trampolines_pool_[info.GetOpCode()](info);
        } catch (const std::bad_any_cast& e) {
            CHECK_FAILED("typecheck: Bad any cast in asynchronous rendering operations");
        } catch (const std::exception& e) {
            info.SetReturnStatus(PresentRemoteCall::Status::kCaught);
            info.SetCaughtException(e.what());
        }
    }
    info.SetThis(nullptr);
}

void PresentRemoteHandle::SetMethodTrampoline(OpCode opcode, RemoteCallTrampoline func)
{
    if (opcode >= trampolines_pool_size_)
    {
        OpCode oldSize = trampolines_pool_size_;
        trampolines_pool_size_ = opcode + 8;
        trampolines_pool_ = reinterpret_cast<RemoteCallTrampoline*>(
                realloc(trampolines_pool_, sizeof(OpCode) * trampolines_pool_size_));
        CHECK(trampolines_pool_);
        bzero(trampolines_pool_ + oldSize, sizeof(OpCode) * (oldSize - trampolines_pool_size_));
    }
    trampolines_pool_[opcode] = func;
}

void PresentRemoteHandle::Invoke(PresentRemoteCall info, const PresentRemoteCallResultCallback& callback)
{
    PresentThread *present_thread = GlobalScope::Ref().GetPresentThread();
    CHECK(present_thread);
    present_thread->EnqueueRemoteCall(Self(), std::move(info), callback);
}

void PresentRemoteHandle::Emit(SignalCode signal, PresentSignal info)
{
    signal_slots_map_lock_.lock();
    if (signal_slots_map_.count(signal) == 0)
    {
        signal_slots_map_lock_.unlock();
        return;
    }
    signal_slots_map_lock_.unlock();

    bool has_local_listeners = false;
    signal_slots_map_lock_.lock();
    for (const auto& slot : signal_slots_map_)
    {
        if (slot.second.local_thread)
            has_local_listeners = true;
    }
    signal_slots_map_lock_.unlock();

    auto *local_context = PresentThread::LocalContext::GetCurrent();
    local_context->EnqueueSignal(Self(), signal, std::move(info), has_local_listeners);
}

uint32_t PresentRemoteHandle::Connect(SignalCode signal, const PresentSignalCallback& callback,
                                     bool localThread)
{
    std::scoped_lock lock(signal_slots_map_lock_);
    uint32_t slotId = ++slot_id_counter_;
    signal_slots_map_.insert(std::make_pair(signal, ConnectedSlot{slotId, callback, localThread}));
    return slotId;
}

void PresentRemoteHandle::Disconnect(uint32_t id)
{
    std::scoped_lock lock(signal_slots_map_lock_);
    for (auto itr = signal_slots_map_.begin(); itr != signal_slots_map_.end(); itr++)
    {
        if (itr->second.id == id)
        {
            signal_slots_map_.erase(itr);
            break;
        }
    }
}

void PresentRemoteHandle::DoEmitSignal(SignalCode signal_code,
                                       PresentSignal& signal_info,
                                       bool local_thread)
{
    std::optional<std::scoped_lock<std::mutex>> maybe_scoped_lock;
    if (local_thread)
        maybe_scoped_lock.emplace(signal_slots_map_lock_);

    size_t count = signal_slots_map_.count(signal_code);

    if (count == 0)
        return;

    std::vector<PresentSignalCallback> callbacks;

    auto itr = signal_slots_map_.lower_bound(signal_code);
    while (itr != signal_slots_map_.end() && itr->first == signal_code)
    {
        if (itr->second.local_thread == local_thread)
            callbacks.emplace_back(itr->second.callback);
        itr++;
    }

    for (const PresentSignalCallback& slot : callbacks)
    {
        PresentSignalArgs signal_args(signal_info);
        slot(signal_args);
    }
}

namespace {

#define E(x) { PresentRemoteHandle::RealType::k##x, #x }
struct {
    PresentRemoteHandle::RealType type;
    const char *name;
} g_object_type_name_map[] = {
    E(TaskRunner),
    E(Display),
    E(Surface),
    E(ContentAggregator),
    E(Monitor),
    E(Cursor),
    E(CursorTheme)
};

#undef E
} // namespace anonymous

const char *PresentRemoteHandle::GetTypeName(RealType type)
{
    for (const auto& pair : g_object_type_name_map)
    {
        if (pair.type == type)
            return pair.name;
    }
    MARK_UNREACHABLE("Unexpected enumeration value of `RealType`");
}

GLAMOR_NAMESPACE_END
