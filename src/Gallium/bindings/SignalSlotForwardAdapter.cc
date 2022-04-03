#include "Gallium/bindings/SignalSlotForwardAdapter.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_NS_BEGIN

void SignalSlotForwardAdapter::RegisterClass(v8::Isolate *isolate)
{
    binder::Class<SignalSlotForwardAdapter>(isolate)
            .constructor<>()
            .set("connect", &SignalSlotForwardAdapter::connect)
            .set("disconnect", &SignalSlotForwardAdapter::disconnect)
            .set("clearConnections", &SignalSlotForwardAdapter::clearConnections);
}

SignalSlotForwardAdapter::~SignalSlotForwardAdapter()
{
    clearConnections();
}

void SignalSlotForwardAdapter::clearConnections()
{
    fConnections.clear();
    for (auto& pair : fInvocations)
        pair.second.Reset();
    fInvocations.clear();
}

namespace {
uint32_t connection_id_counter_ = 1;
} // namespace anonymous

uint32_t SignalSlotForwardAdapter::connect(const std::string& signal, v8::Local<v8::Function> slot)
{
    uint32_t id = connection_id_counter_++;
    fConnections.insert({signal, id});
    v8::Global<v8::Function> cb(v8::Isolate::GetCurrent(), slot);
    fInvocations.insert({id, std::move(cb)});
    return id;
}

void SignalSlotForwardAdapter::disconnect(uint32_t id)
{
    bool found = false;
    for (auto itr = fConnections.begin(); itr != fConnections.end(); itr++)
    {
        if (itr->second == id)
        {
            fConnections.erase(itr);
            found = true;
            break;
        }
    }
    if (!found)
        return;

    for (auto itr = fInvocations.begin(); itr != fInvocations.end(); itr++)
    {
        if (itr->first == id)
        {
            itr->second.Reset();
            fInvocations.erase(itr);
            return;
        }
    }
}

GALLIUM_BINDINGS_NS_END
