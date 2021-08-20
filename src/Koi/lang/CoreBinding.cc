#include <iostream>
#include <optional>

#include "include/v8.h"

#include "Core/Properties.h"
#include "Core/EventLoop.h"
#include "Core/EventSource.h"
#include "Core/Journal.h"

#include "Koi/binder/Class.h"
#include "Koi/binder/Module.h"
#include "Koi/binder/CallV8.h"
#include "Koi/lang/CoreBinding.h"
KOI_LANG_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.lang)

namespace {

/* ----------------- Utilities  ----------------- */
std::optional<int> parse_property_array_subscript(const std::string_view& spec,
                                                  bool& thrown,
                                                  v8::Isolate *isolate)
{
    thrown = false;
    if (!spec.starts_with('#'))
        return {};

    std::string_view subscript(spec);
    subscript.remove_prefix(1);

    std::string subscript_dump(subscript);

    char *end_ptr = nullptr;
    long subscript_val = std::strtol(subscript_dump.c_str(), &end_ptr, 10);
    if (end_ptr - subscript_dump.c_str() != subscript_dump.size())
    {
        binder::throw_(isolate, "Array subscript should be an integer");
        thrown = true;
        return {};
    }

    if (subscript_val < 0)
    {
        binder::throw_(isolate, "Array subscript should be a positive integer");
        thrown = true;
        return {};
    }

    if (subscript_val > INT_MAX)
    {
        binder::throw_(isolate, "Array subscript is too large");
        thrown = true;
        return {};
    }

    return std::make_optional(static_cast<int>(subscript_val));
}

std::shared_ptr<PropertyNode> parse_property_spec(const std::string& spec,
                                                  bool& thrown,
                                                  v8::Isolate *isolate)
{
    thrown = false;

    std::vector<std::string_view> selectors;
    size_t p = 0;
    int64_t last_p = -1;
    while ((p = spec.find('.', p + 1)) != std::string::npos)
    {
        std::string_view view(spec);
        view.remove_prefix(last_p + 1);
        view.remove_suffix(spec.size() - p);
        selectors.emplace_back(view);
        last_p = static_cast<int64_t>(p);
    }
    std::string_view view(spec);
    view.remove_prefix(last_p + 1);
    selectors.emplace_back(view);

    std::shared_ptr<PropertyNode> currentNode = prop::Get();
    for (auto const& sel : selectors)
    {
        if (currentNode->kind() == PropertyNode::Kind::kData)
            return nullptr;

        auto maybe_array = parse_property_array_subscript(sel, thrown, isolate);
        if (thrown)
            return nullptr;

        if (maybe_array)
        {
            if (currentNode->kind() != PropertyNode::Kind::kArray)
            {
                binder::throw_(isolate, "Illegal usage of array subscript");
                thrown = true;
                return nullptr;
            }

            int subscript = maybe_array.value();
            auto array_node = prop::Cast<PropertyArrayNode>(currentNode);
            if (subscript >= array_node->size())
                return nullptr;

            currentNode = array_node->at(subscript);
        }
        else
        {
            if (currentNode->kind() != PropertyNode::Kind::kObject)
            {
                binder::throw_(isolate, "Illegal usage of member selector");
                thrown = true;
                return nullptr;
            }

            auto object_node = prop::Cast<PropertyObjectNode>(currentNode);
            if (!object_node->hasMember(std::string(sel)))
                return nullptr;

            currentNode = object_node->getMember(std::string(sel));
        }
    }

    return currentNode;
}

/* ----------------- Native Functions ----------------- */

void jni_core_print(const std::string& str)
{
    if (str.empty())
        return;
    std::fwrite(str.c_str(), str.size(), 1, stdout);
}

class jni_core_DelayTimer : public TimerSource
{
public:
    jni_core_DelayTimer(uint64_t timeout,
                        v8::Isolate *isolate,
                        v8::Local<v8::Promise::Resolver> resolver)
        : TimerSource(EventLoop::Instance()),
          fIsolate(isolate),
          fResolve(fIsolate, resolver)
    {
        startTimer(timeout);
    }
    ~jni_core_DelayTimer() override
    {
        fResolve.Reset();
    }

private:
    KeepInLoop timerDispatch() override
    {
        v8::Local<v8::Promise::Resolver> resolve = fResolve.Get(fIsolate);
        resolve->Resolve(fIsolate->GetCurrentContext(), v8::Null(fIsolate))
            .Check();
        delete this;
        return KeepInLoop::kDeleted;
    }

    v8::Isolate *fIsolate;
    v8::Global<v8::Promise::Resolver>   fResolve;
};

v8::Local<v8::Value> jni_core_delay(uint64_t timeout)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();

    /* The object will be deleted in jni_core_DelayTimer::timerDispatch(). */
    new jni_core_DelayTimer(timeout, isolate, resolver);
    return resolver->GetPromise();
}

v8::Local<v8::Value> jni_core_getProperty(const std::string& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    bool thrown = false;
    std::shared_ptr<PropertyNode> maybeNode = parse_property_spec(spec, thrown, isolate);
    if (thrown)
        return {};

    if (!maybeNode || maybeNode->protection() == PropertyNode::Protection::kPrivate)
    {
        binder::throw_(isolate, "No such property or property is inaccessible to JavaScript");
        return {};
    }

    if (maybeNode->kind() != PropertyNode::Kind::kData)
    {
        binder::throw_(isolate, "Property is an array or object");
        return {};
    }

    auto node = prop::Cast<PropertyDataNode>(maybeNode);
#define T_(t)  (node->type() == typeid(t))
    if (T_(int8_t) || T_(int16_t) || T_(int32_t))
        return v8::Integer::New(isolate, node->extract<int32_t>());
    else if (T_(uint8_t) || T_(uint16_t) || T_(uint32_t))
        return v8::Integer::NewFromUnsigned(isolate, node->extract<uint32_t>());
    else if (T_(int64_t))
        return v8::BigInt::New(isolate, node->extract<int64_t>());
    else if (T_(uint64_t))
        return v8::BigInt::NewFromUnsigned(isolate, node->extract<uint64_t>());
    else if (T_(bool))
        return v8::Boolean::New(isolate, node->extract<bool>());
    else if (T_(float))
        return v8::Number::New(isolate, node->extract<float>());
    else if (T_(double))
        return v8::Number::New(isolate, node->extract<double>());
    else if (T_(const char*))
        return v8::String::NewFromUtf8(isolate, node->extract<const char*>()).ToLocalChecked();
    else if (T_(std::string))
        return v8::String::NewFromUtf8(isolate, node->extract<std::string>().c_str()).ToLocalChecked();
#undef T_

    binder::throw_(isolate, "Property is not of primitive type");
    return {};
}

bool jni_core_hasProperty(const std::string& spec)
{
    bool thrown = false;
    std::shared_ptr<PropertyNode> node = parse_property_spec(spec, thrown, v8::Isolate::GetCurrent());
    if (thrown)
        return {};

    return (!node || node->protection() == PropertyNode::Protection::kPrivate);
}

/* ----------------- Native Classes ----------------- */
class jni_core_Timer : public TimerSource
{
public:
    jni_core_Timer()
        : TimerSource(EventLoop::Instance()) {}
    ~jni_core_Timer() override = default;

    static binder::Class<jni_core_Timer> GetClass()
    {
        return binder::Class<jni_core_Timer>(v8::Isolate::GetCurrent())
                .constructor<>()
                .set("setInterval", &jni_core_Timer::setInterval)
                .set("setTimeout", &jni_core_Timer::setTimeout)
                .set("stop", &jni_core_Timer::stop);
    }

    void setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue)
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        if (!cbValue->IsFunction())
        {
            binder::throw_(isolate, "Callback should be a function");
            return;
        }
        fCallback = v8::Global<v8::Function>(isolate, v8::Local<v8::Function>::Cast(cbValue));
        TimerSource::startTimer(timeout, repeat);
    }

    void setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue)
    {
        setInterval(timeout, 0, cbValue);
    }

    void stop()
    {
        TimerSource::stopTimer();
        fCallback.Reset();
    }

private:
    KeepInLoop timerDispatch() override
    {
        if (fCallback.IsEmpty())
        {
            LOGW(LOG_WARNING, "A timer was dispatched but the callback function is empty")
            return KeepInLoop::kNo;
        }
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        v8::Local<v8::Value> result = binder::call_v8(isolate,
                                                      fCallback.Get(isolate),
                                                      isolate->GetCurrentContext()->Global());

        if (result->IsBoolean())
        {
            bool ret = result->ToBoolean(isolate)->Value();
            return ret ? KeepInLoop::kYes : KeepInLoop::kNo;
        }
        return KeepInLoop::kYes;
    }

    v8::Global<v8::Function> fCallback;
};

} // namespace anonymous

class CoreBindingModule : public BaseBindingModule
{
public:
    CoreBindingModule()
        : BaseBindingModule("core",
                            "Basic language features for Cocoa JavaScript") {}
    ~CoreBindingModule() override = default;

    void getModule(binder::Module& self) override
    {
        auto jni_core_Timer_class = jni_core_Timer::GetClass();
        self.set("print", jni_core_print)
            .set("delay", jni_core_delay)
            .set("getProperty", jni_core_getProperty)
            .set("hasProperty", jni_core_hasProperty)
            .set("Timer", jni_core_Timer_class);
    }
};

KOI_DECL_BINDING_PRELOAD_HOOK(core)
{
    RegisterBinding(new CoreBindingModule());
}

KOI_LANG_NS_END
