#ifndef COCOA_SIGNALSLOTFORWARDADAPTER_H
#define COCOA_SIGNALSLOTFORWARDADAPTER_H

#include <map>
#include <sigc++/sigc++.h>

#include "Koi/bindings/Base.h"
#include "Koi/binder/CallV8.h"
KOI_BINDINGS_NS_BEGIN

class SignalSlotForwardAdapter : public sigc::trackable
{
public:
    SignalSlotForwardAdapter() = default;
    ~SignalSlotForwardAdapter();

    static void RegisterClass(v8::Isolate *isolate);

    void clearConnections();
    uint32_t connect(const std::string& signal, v8::Local<v8::Function> slot);
    void disconnect(uint32_t id);

protected:
    template<typename...ArgsT>
    void ReEmit(const std::string& signal, ArgsT&&...args) {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        auto idIterator = fConnections.find(signal);
        if (idIterator == fConnections.end())
            return;
        v8::HandleScope scope(isolate);
        v8::Local<v8::Object> global = isolate->GetCurrentContext()->Global();
        for (; idIterator->first == signal; idIterator++)
        {
            CHECK(fInvocations.count(idIterator->second) && "Corrupted callback ID");
            v8::Local<v8::Function> func = fInvocations[idIterator->second].Get(isolate);
            binder::Invoke(isolate, func, global, std::forward<ArgsT>(args)...);
        }
    }

private:
    std::map<uint32_t, v8::Global<v8::Function>> fInvocations;
    std::multimap<std::string, uint32_t>         fConnections;
};

KOI_BINDINGS_NS_END
#endif //COCOA_SIGNALSLOTFORWARDADAPTER_H
