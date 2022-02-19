#include "Core/Journal.h"
#include "Koi/bindings/cobalt/Exports.h"
#include "Koi/Runtime.h"

#include "Cobalt/RenderHost.h"

#include <utility>
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHostCreator.h"
#include "Cobalt/Display.h"
KOI_BINDINGS_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.bindings.Cobalt)

namespace i = cobalt;

void RenderHostInitialize(const std::string& application)
{
    i::GlobalScope::Ref().Initialize();
    QLOG(LOG_INFO, "RenderHost is initialized, application name %fg<gr>\"{}\"%reset", application);
}

void RenderHostDispose()
{
    i::GlobalScope::Ref().Dispose();
    QLOG(LOG_INFO, "RenderHost is disposed");
}

struct PromiseClosure
{
    using InfoConverter = v8::Local<v8::Value>(*)(v8::Isolate*, i::RenderHostCallbackInfo&);

    PromiseClosure(v8::Isolate *isolate, InfoConverter conv)
        : isolate_(isolate)
        , info_converter_(conv)
    {
        v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
        auto resolver = v8::Promise::Resolver::New(ctx).ToLocalChecked();
        resolver_ = v8::Global<v8::Promise::Resolver>(isolate, resolver);
    }

    ~PromiseClosure()
    {
        resolver_.Reset();
    }

    static std::shared_ptr<PromiseClosure> New(v8::Isolate *isolate, InfoConverter converter)
    {
        return std::make_shared<PromiseClosure>(isolate, converter);
    }

    bool rejectIfEssential(i::RenderHostCallbackInfo& info)
    {
        std::string reason;
        v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
        switch (info.GetReturnStatus())
        {
        case i::RenderClientCallInfo::Status::kOpCodeInvalid:
            reason = "Invalid operation code";
            break;
        case i::RenderClientCallInfo::Status::kArgsInvalid:
            reason = "Invalid arguments";
            break;
        case i::RenderClientCallInfo::Status::kCaught:
            reason = info.GetCaughtException().what();
            break;
        case i::RenderClientCallInfo::Status::kOpFailed:
            reason = "Operation failed";
            break;
        default:
            return false;
        }
        auto error = v8::Exception::Error(binder::to_v8(isolate_, reason)).As<v8::Object>();
        error->Set(ctx, binder::to_v8(isolate_, "opcode"),
                   binder::to_v8(isolate_, info.GetOpcode())).Check();
        resolver_.Get(isolate_)->Reject(ctx, error).Check();
        return true;
    }

    static void HostCallback(i::RenderHostCallbackInfo& info) {
        auto this_ = info.GetClosure<std::shared_ptr<PromiseClosure>>();

        v8::HandleScope scope(this_->isolate_);
        if (this_->rejectIfEssential(info))
            return;

        v8::Local<v8::Promise::Resolver> r = this_->resolver_.Get(this_->isolate_);
        v8::Local<v8::Context> ctx = this_->isolate_->GetCurrentContext();
        if (info.HasReturnValue())
        {
            r->Resolve(ctx, this_->info_converter_(this_->isolate_, info)).Check();
        }
        else
        {
            r->Resolve(ctx, v8::Undefined(this_->isolate_)).Check();
        }
    }

    v8::Local<v8::Promise> getPromise()
    {
        return resolver_.Get(isolate_)->GetPromise();
    }

    template<typename Wrapper, typename T>
    static v8::Local<v8::Value> CreateObjectConverter(v8::Isolate *isolate,
                                                      i::RenderHostCallbackInfo& info)
    {
        return binder::Class<Wrapper>::create_object(isolate, info.GetReturnValue<T>());
    }

    v8::Isolate *isolate_;
    v8::Global<v8::Promise::Resolver> resolver_;
    InfoConverter info_converter_;
};

v8::Local<v8::Value> Connect(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (info.Length() > 1)
    {
        binder::JSException::Throw(binder::ExceptT::kError,
                                   "Invalid number of arguments, expecting 0 or 1 argument");
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

struct RenderClientObjectWrap::SlotClosure
{
    using Converter = v8::Local<v8::Value>(*)(std::any&);

    SlotClosure(v8::Isolate *isolate, v8::Local<v8::Function> callback,
                int32_t signal, v8::Local<v8::Object> wrap_object,
                const i::co_sp<i::RenderClientObject>& object,
                Converter *value_converters, size_t n_value_converters)
        : isolate_(isolate)
        , render_host_object_wrap_(isolate_, wrap_object)
        , callback_(isolate_, callback)
        , slot_id_(0)
        , value_converters_(value_converters)
        , n_value_converters_(n_value_converters)
    {
        slot_id_ = object->Connect(signal, [this](i::RenderHostSlotCallbackInfo& info) {
            v8::HandleScope scope(isolate_);
            CHECK(info.Length() == n_value_converters_);
            v8::Local<v8::Function> func = callback_.Get(isolate_);
            v8::Local<v8::Value> *values = nullptr;

            if (n_value_converters_ > 0)
            {
                values = new v8::Local<v8::Value>[n_value_converters_];
                for (size_t i = 0; i < n_value_converters_; i++)
                    values[i] = value_converters_[i](info.Get(i));
            }

            v8::TryCatch catchScope(isolate_);
            func->Call(isolate_->GetCurrentContext(),
                       isolate_->GetCurrentContext()->Global(),
                       static_cast<int>(info.Length()),
                       values).ToLocalChecked();
            if (catchScope.HasCaught())
                Runtime::GetBareFromIsolate(isolate_)->reportUncaughtExceptionInCallback(catchScope);

            delete[] values;
        });
    }

    ~SlotClosure()
    {
        render_host_object_wrap_.Reset();
        callback_.Reset();
    }

    v8::Isolate *isolate_;
    /* hold a reference here to avoid being destroyed by GC */
    v8::Global<v8::Object> render_host_object_wrap_;
    v8::Global<v8::Function> callback_;
    uint32_t slot_id_;
    Converter *value_converters_;
    size_t n_value_converters_;
};

RenderClientObjectWrap::RenderClientObjectWrap(i::co_sp<i::RenderClientObject> object)
    : object_(std::move(object))
{
}

RenderClientObjectWrap::~RenderClientObjectWrap()
{
    for (auto& pair : slot_closures_map_)
    {
        getObject()->Disconnect(pair.second->slot_id_);
        delete pair.second;
    }
}

void RenderClientObjectWrap::setSignalName(const char *name, int32_t code)
{
    signal_name_map_[name] = code;
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
        binder::JSException::Throw(binder::ExceptT::kError,
                                   "Invalid signal name for slot to connect to");
    }
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *closure = new SlotClosure(isolate, callback, code,
                                    binder::Class<RenderClientObjectWrap>::find_object(isolate, this),
                                    getObject(),
                                    nullptr,
                                    0);
    CHECK(closure);
    slot_closures_map_[closure->slot_id_] = closure;

    return closure->slot_id_;
}

void RenderClientObjectWrap::disconnect(uint32_t id)
{
    if (slot_closures_map_.count(id) == 0)
    {
        binder::JSException::Throw(binder::ExceptT::kError, "Invalid slot ID");
    }

    getObject()->Disconnect(id);
    delete slot_closures_map_[id];
    slot_closures_map_.erase(id);
}

DisplayWrap::DisplayWrap(cobalt::co_sp<cobalt::RenderClientObject> object)
    : RenderClientObjectWrap(std::move(object))
{
    setSignalName("closed", CRSI_DISPLAY_CLOSED);
}

DisplayWrap::~DisplayWrap() = default;

v8::Local<v8::Value> DisplayWrap::close()
{
    auto closure = PromiseClosure::New(v8::Isolate::GetCurrent(), nullptr);
    getObject()->Invoke(CROP_DISPLAY_CLOSE,
                        closure,
                        PromiseClosure::HostCallback);
    return closure->getPromise();
}

KOI_BINDINGS_NS_END
