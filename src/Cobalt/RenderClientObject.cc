#include "Core/Journal.h"

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHost.h"
#include "Cobalt/RenderClientSignalEmit.h"
COBALT_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Cobalt.RenderClientObject)

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

    auto *emit = new RenderClientSignalEmit(std::move(info), shared_from_this(), signal);
    CHECK(emit);

    host->WakeupHost(emit);
}

uint32_t RenderClientObject::Connect(SignalCode signal, const RenderHostSlotCallback& callback)
{
    std::scoped_lock lock(signal_slots_map_lock_);
    uint32_t slotId = ++slot_id_counter_;
    signal_slots_map_.insert(std::make_pair(signal, ConnectedSlot{slotId, callback}));
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

void RenderClientObject::EmitterTrampoline(RenderClientSignalEmit *emit)
{
    SignalCode signal = emit->GetSignalCode();
    size_t count = signal_slots_map_.count(signal);

    if (count == 0)
        return;

    std::vector<RenderHostSlotCallback> callbacks;

    auto itr = signal_slots_map_.lower_bound(signal);
    while (itr != signal_slots_map_.end() && itr->first == signal)
    {
        callbacks.emplace_back(itr->second.callback);
        itr++;
    }

    for (const RenderHostSlotCallback& slot : callbacks)
    {
        RenderHostSlotCallbackInfo info(emit);
        slot(info);
    }
}

COBALT_NAMESPACE_END
