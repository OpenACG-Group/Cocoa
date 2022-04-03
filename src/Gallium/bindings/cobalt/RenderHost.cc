#include "Core/Journal.h"
#include "Gallium/bindings/cobalt/Exports.h"
#include "Gallium/bindings/cobalt/PromiseHelper.h"
#include "Gallium/Runtime.h"

#include "Cobalt/RenderHost.h"

#include <utility>
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHostCreator.h"
GALLIUM_BINDINGS_COBALT_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.Cobalt)

namespace i = ::cocoa::cobalt;

void RenderHostWrap::Initialize(v8::Local<v8::Object> info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    for (const char *field : {"name", "major", "minor", "patch"})
    {
        if (!info->Has(ctx, binder::to_v8(isolate, field)).FromMaybe(false))
            g_throw(TypeError, fmt::format("Missing \"{}\" field in ApplicationInfo", field));
    }

    i::GlobalScope::ApplicationInfo appInfo{};
    appInfo.name =
        binder::from_v8<std::string>(isolate, info->Get(ctx, binder::to_v8(isolate, "name")).ToLocalChecked());
    std::get<0>(appInfo.version_triple) =
        binder::from_v8<int32_t>(isolate, info->Get(ctx, binder::to_v8(isolate, "major")).ToLocalChecked());
    std::get<1>(appInfo.version_triple) =
            binder::from_v8<int32_t>(isolate, info->Get(ctx, binder::to_v8(isolate, "minor")).ToLocalChecked());
    std::get<2>(appInfo.version_triple) =
            binder::from_v8<int32_t>(isolate, info->Get(ctx, binder::to_v8(isolate, "patch")).ToLocalChecked());

    i::GlobalScope::Ref().Initialize(appInfo);
    QLOG(LOG_INFO, "RenderHost is initialized, application name %fg<gr>\"{}\"%reset", appInfo.name);
}

void RenderHostWrap::Dispose()
{
    i::GlobalScope::Ref().Dispose();
    QLOG(LOG_INFO, "RenderHost is disposed");
}

v8::Local<v8::Value> RenderHostWrap::Connect(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (info.Length() > 1)
    {
        g_throw(Error, "Invalid number of arguments, expecting 0 or 1 argument");
    }

    std::string name;
    if (info.Length() == 1)
        name = binder::from_v8<std::string>(isolate, info[0]);

    auto creator = i::GlobalScope::Ref().GetRenderHost()->GetRenderHostCreator();

    using W = DisplayWrap;
    using T = i::co_sp<i::RenderClientObject>;
    auto pack = PromiseClosure::New(isolate, PromiseClosure::CreateObjectConverter<W, T>);

    creator->Invoke(CROP_RENDERHOSTCREATOR_CREATE_DISPLAY,
                    pack,
                    PromiseClosure::HostCallback,
                    name);

    return pack->getPromise();
}

// ============================
// RenderClientObjectWrap
// ============================

RenderClientObjectWrap::RenderClientObjectWrap(i::co_sp<i::RenderClientObject> object)
    : object_(std::move(object))
{
}

RenderClientObjectWrap::~RenderClientObjectWrap()
{
    slot_closures_map_.clear();
}

void RenderClientObjectWrap::defineSignal(const char *name, int32_t code, InfoAcceptor acceptor)
{
    CHECK(signal_name_map_.count(name) == 0 || acceptors_map_.count(code) == 0);
    signal_name_map_[name] = code;
    acceptors_map_[code] = acceptor;
}

int32_t RenderClientObjectWrap::getSignalCodeByName(const std::string& name)
{
    if (signal_name_map_.count(name) == 0)
        return -1;
    return signal_name_map_[name];
}

uint32_t RenderClientObjectWrap::connect(const std::string& name, v8::Local<v8::Function> callback)
{
    int32_t code = getSignalCodeByName(name);
    if (code < 0)
    {
        g_throw(Error, "Invalid signal name for slot to connect to");
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto closure = SlotClosure::New(isolate, code, getObject(), callback, acceptors_map_[code]);
    uint32_t slotId = closure->slot_id_;
    slot_closures_map_[closure->slot_id_] = std::move(closure);

    return slotId;
}

void RenderClientObjectWrap::disconnect(uint32_t id)
{
    if (slot_closures_map_.count(id) == 0)
    {
        g_throw(Error, "Invalid slot ID");
    }
    slot_closures_map_.erase(id);
}

GALLIUM_BINDINGS_COBALT_NS_END
