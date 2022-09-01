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
#include "Glamor/RenderClientObject.h"
#include "Glamor/RenderHost.h"
#include "Glamor/RenderClientSignalEmit.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.RenderClientObject)

RenderClientObject::RenderClientObject(RealType type)
    : real_type_(type)
    , trampolines_pool_(nullptr)
    , trampolines_pool_size_(kTrampolinePoolInitSize)
    , slot_id_counter_(0)
    , dangling_callbacks_counter_(0)
{
    trampolines_pool_ = reinterpret_cast<RenderClientCallTrampoline*>(
            calloc(sizeof(OpCode), kTrampolinePoolInitSize));
    CHECK(trampolines_pool_);

    dummy_host_callback_ = [this](RenderHostCallbackInfo& info) {
        this->dangling_callbacks_counter_++;
    };
}

RenderClientObject::~RenderClientObject()
{
    if (trampolines_pool_)
        free(trampolines_pool_);
}

void RenderClientObject::CallFromHostTrampoline(RenderClientCallInfo& info)
{
    info.SetThis(shared_from_this());
    if (info.GetOpCode() >= trampolines_pool_size_ || !trampolines_pool_[info.GetOpCode()])
    {
        info.SetReturnStatus(RenderClientCallInfo::Status::kOpCodeInvalid);
    }
    else
    {
        try {
            trampolines_pool_[info.GetOpCode()](info);
        } catch (const std::exception& e) {
            info.SetReturnStatus(RenderClientCallInfo::Status::kCaught);
            info.SetCaughtException(e);
        }
    }
    info.SetThis(nullptr);
}

void RenderClientObject::SetMethodTrampoline(OpCode opcode, RenderClientCallTrampoline func)
{
    if (opcode >= trampolines_pool_size_)
    {
        OpCode oldSize = trampolines_pool_size_;
        trampolines_pool_size_ = opcode + 8;
        trampolines_pool_ = reinterpret_cast<RenderClientCallTrampoline*>(
                realloc(trampolines_pool_, sizeof(OpCode) * trampolines_pool_size_));
        CHECK(trampolines_pool_);
        bzero(trampolines_pool_ + oldSize, sizeof(OpCode) * (oldSize - trampolines_pool_size_));
    }
    trampolines_pool_[opcode] = func;
}

void RenderClientObject::Invoke(RenderClientCallInfo info, const RenderHostCallback& callback)
{
    RenderHost *host = GlobalScope::Ref().GetRenderHost();
    host->Send(shared_from_this(), std::move(info), callback);
}

void RenderClientObject::Emit(SignalCode signal, RenderClientEmitterInfo info)
{
    signal_slots_map_lock_.lock();
    if (signal_slots_map_.count(signal) == 0)
    {
        signal_slots_map_lock_.unlock();
        return;
    }
    signal_slots_map_lock_.unlock();

    RenderHost *host = GlobalScope::Ref().GetRenderHost();
    RenderClient *client = GlobalScope::Ref().GetRenderClient();

    auto emit = std::make_shared<RenderClientSignalEmit>(std::move(info), shared_from_this(), signal);

    // Check whether there are slots which should be called on render thread
    bool hasLocalThreadSlots = false;
    signal_slots_map_lock_.lock();
    for (const auto& slot : signal_slots_map_)
    {
        if (slot.second.local_thread)
            hasLocalThreadSlots = true;
    }
    signal_slots_map_lock_.unlock();

    // If there are, call them later
    if (hasLocalThreadSlots)
        client->ScheduleDeferredLocalThreadSlotsInvocation(emit, Self());

    emit->MarkProfileMilestone(ITCProfileMilestone::kClientEmitted);
    host->WakeupHost(emit);
}

uint32_t RenderClientObject::Connect(SignalCode signal, const RenderHostSlotCallback& callback,
                                     bool localThread)
{
    std::scoped_lock lock(signal_slots_map_lock_);
    uint32_t slotId = ++slot_id_counter_;
    signal_slots_map_.insert(std::make_pair(signal, ConnectedSlot{slotId, callback, localThread}));
    return slotId;
}

void RenderClientObject::Disconnect(uint32_t id)
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

void RenderClientObject::EmitterTrampoline(const std::shared_ptr<RenderClientSignalEmit>& emit, bool localThread)
{
    std::optional<std::scoped_lock<std::mutex>> maybeScopedLock;
    if (localThread)
        maybeScopedLock.emplace(signal_slots_map_lock_);

    SignalCode signal = emit->GetSignalCode();
    size_t count = signal_slots_map_.count(signal);

    if (count == 0)
        return;

    std::vector<RenderHostSlotCallback> callbacks;

    auto itr = signal_slots_map_.lower_bound(signal);
    while (itr != signal_slots_map_.end() && itr->first == signal)
    {
        if (itr->second.local_thread == localThread)
            callbacks.emplace_back(itr->second.callback);
        itr++;
    }

    for (const RenderHostSlotCallback& slot : callbacks)
    {
        RenderHostSlotCallbackInfo info(emit.get());
        slot(info);
    }
}

namespace {

#define E(x) { RenderClientObject::RealType::k##x, #x }
struct {
    RenderClientObject::RealType type;
    const char *name;
} g_object_type_name_map[] = {
    E(RenderHostTaskRunner),
    E(RenderHostCreator),
    E(Display),
    E(Surface),
    E(Blender),
    E(Monitor),
    E(Cursor),
    E(CursorTheme)
};

#undef E
} // namespace anonymous

const char *RenderClientObject::GetTypeName(RealType type)
{
    for (const auto& pair : g_object_type_name_map)
    {
        if (pair.type == type)
            return pair.name;
    }
    MARK_UNREACHABLE("Unexpected enumeration value of `RealType`");
}

GLAMOR_NAMESPACE_END
