#include <iostream>
#include <optional>
#include "include/v8.h"

#include "Core/Properties.h"
#include "Core/EventLoop.h"
#include "Core/EventSource.h"
#include "Core/Journal.h"
#include "Core/Filesystem.h"
#include "Core/CrpkgImage.h"

#include "Koi/binder/Class.h"
#include "Koi/binder/Module.h"
#include "Koi/binder/CallV8.h"
#include "Koi/binder/Property.h"
#include "Koi/lang/Base.h"
#include "Koi/lang/CoreBinding.h"
#include "Koi/lang/CoreFDLR.h"
KOI_LANG_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.lang)

/////////////////////////////////////////////////////////
// Utilities
//
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

/////////////////////////////////////////////////////////
// Cocoa.core.print()
//

void jni_core_print(const std::string& str)
{
    if (str.empty())
        return;
    std::fwrite(str.c_str(), str.size(), 1, stdout);
}

/////////////////////////////////////////////////////////
// Cocoa.core.delay()
//

class DelayTimer : public TimerSource
{
public:
    DelayTimer(uint64_t timeout,
               v8::Isolate *isolate,
               v8::Local<v8::Promise::Resolver> resolver)
        : TimerSource(EventLoop::Instance()),
          fIsolate(isolate),
          fResolve(fIsolate, resolver)
    {
        startTimer(timeout);
    }
    ~DelayTimer() override
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

    /* The object will be deleted in DelayTimer::timerDispatch(). */
    new DelayTimer(timeout, isolate, resolver);
    return resolver->GetPromise();
}

/////////////////////////////////////////////////////////
// Cocoa.core.getProperty(), Cocoa.core.hasProperty()
//
namespace
{
std::map<PropertyNode::Kind, const char*> gNodeKindNameMap = {
        {PropertyNode::Kind::kData, "data"},
        {PropertyNode::Kind::kObject, "object"},
        {PropertyNode::Kind::kArray, "array"}
};
} // namespace anonymous

v8::Local<v8::Value> jni_core_getProperty(const std::string& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    bool thrown = false;
    std::shared_ptr<PropertyNode> maybeNode = parse_property_spec(spec, thrown, isolate);
    if (thrown)
        return {};

    if (!maybeNode || maybeNode->protection() == PropertyNode::Protection::kPrivate)
    {
        binder::throw_(isolate, "No such property or inaccessible to JavaScript");
        return {};
    }
    auto node = prop::Cast<PropertyDataNode>(maybeNode);

    v8::Local<v8::Object> result = v8::Object::New(isolate);
    result->Set(context, binder::to_v8(isolate, "type"),
                binder::to_v8(isolate, gNodeKindNameMap[node->kind()])).Check();

    if (maybeNode->kind() != PropertyNode::Kind::kData)
        return scope.Escape(result);

    v8::Local<v8::Primitive> value;
#define T_(t)  (node->type() == typeid(t))
    if (T_(int8_t) || T_(int16_t) || T_(int32_t))
        value = v8::Integer::New(isolate, node->extract<int32_t>());
    else if (T_(uint8_t) || T_(uint16_t) || T_(uint32_t))
        value = v8::Integer::NewFromUnsigned(isolate, node->extract<uint32_t>());
    else if (T_(int64_t))
        value = v8::BigInt::New(isolate, node->extract<int64_t>());
    else if (T_(uint64_t))
        value = v8::BigInt::NewFromUnsigned(isolate, node->extract<uint64_t>());
    else if (T_(bool))
        value = v8::Boolean::New(isolate, node->extract<bool>());
    else if (T_(float))
        value = v8::Number::New(isolate, node->extract<float>());
    else if (T_(double))
        value = v8::Number::New(isolate, node->extract<double>());
    else if (T_(const char*))
        value = v8::String::NewFromUtf8(isolate, node->extract<const char*>()).ToLocalChecked();
    else if (T_(std::string))
        value = v8::String::NewFromUtf8(isolate, node->extract<std::string>().c_str()).ToLocalChecked();
#undef T_
    if (!value.IsEmpty())
        result->Set(context, binder::to_v8(isolate, "value"), value).Check();
    return scope.Escape(result);
}

v8::Local<v8::Value> jni_core_enumeratePropertyNode(const std::string& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    bool thrown = false;
    std::shared_ptr<PropertyNode> node = parse_property_spec(spec, thrown, isolate);
    if (thrown)
        return {};

    if (!node || node->protection() == PropertyNode::Protection::kPrivate)
    {
        binder::throw_(isolate, "No such property or inaccessible to JavaScript");
        return {};
    }
    if (node->kind() == PropertyNode::Kind::kData)
        return v8::Null(isolate);

    v8::Local<v8::Array> resultArray = v8::Array::New(isolate);
    if (node->kind() == PropertyNode::Kind::kObject)
    {
        auto objectNode = prop::Cast<PropertyObjectNode>(node);
        int count = 0;
        for (const auto& child : *objectNode)
        {
            v8::Local<v8::Object> result = v8::Object::New(isolate);
            auto childNode = child.second;
            result->Set(context,
                        binder::to_v8(isolate, "name"),
                        binder::to_v8(isolate, child.first)).Check();
            result->Set(context,
                        binder::to_v8(isolate, "type"),
                        binder::to_v8(isolate, gNodeKindNameMap[childNode->kind()])).Check();
            resultArray->Set(context, count++, result).Check();
        }
    }
    else if (node->kind() == PropertyNode::Kind::kArray)
    {
        auto arrayNode = prop::Cast<PropertyArrayNode>(node);
        int count = 0;
        for (const auto& element : *arrayNode)
        {
            v8::Local<v8::Object> result = v8::Object::New(isolate);
            result->Set(context,
                        binder::to_v8(isolate, "name"),
                        binder::to_v8(isolate, fmt::format("#{}", count))).Check();
            result->Set(context,
                        binder::to_v8(isolate, "type"),
                        binder::to_v8(isolate, gNodeKindNameMap[element->kind()])).Check();
            result->Set(context,
                        binder::to_v8(isolate, "index"),
                        binder::to_v8(isolate, count)).Check();
            resultArray->Set(context, count++, result).Check();
        }
    }
    return handleScope.Escape(resultArray);
}

bool jni_core_hasProperty(const std::string& spec)
{
    bool thrown = false;
    std::shared_ptr<PropertyNode> node = parse_property_spec(spec, thrown, v8::Isolate::GetCurrent());
    if (thrown)
        return {};

    return (node && node->protection() != PropertyNode::Protection::kPrivate);
}

/////////////////////////////////////////////////////////
// jni_core_Timer (Cocoa.core.Timer)
//
jni_core_Timer::jni_core_Timer()
    : TimerSource(EventLoop::Instance())
{
}

binder::Class<jni_core_Timer> jni_core_Timer::GetClass()
{
    return binder::Class<jni_core_Timer>(v8::Isolate::GetCurrent())
        .constructor<>()
        .set("setInterval", &jni_core_Timer::setInterval)
        .set("setTimeout", &jni_core_Timer::setTimeout)
        .set("stop", &jni_core_Timer::stop);
}

void jni_core_Timer::setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue)
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

void jni_core_Timer::setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue)
{
    setInterval(timeout, 0, cbValue);
}

void jni_core_Timer::stop()
{
    TimerSource::stopTimer();
    fCallback.Reset();
}

KeepInLoop jni_core_Timer::timerDispatch()
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

/**
 * Argument #2, Open flags:
 *  r   read only
 *  w   write only
 *  rw  read-write
 *  +   create if not exists
 *  a   append mode
 *  t   truncate
 */
int32_t jni_core_open(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_AND_JS_THROW_WITH_RET(info.Length() < 2, "Too few arguments", -1);
    CHECK_AND_JS_THROW_WITH_RET(info.Length() > 3, "Too much arguments", -1);

    CHECK_AND_JS_THROW_WITH_RET(!info[0]->IsString(), "File path should be a string", -1);
    std::string path = binder::from_v8<std::string>(isolate, info[0]);

    CHECK_AND_JS_THROW_WITH_RET(!info[1]->IsString(), "Open flags should be a string", -1);
    std::string strFlags = binder::from_v8<std::string>(isolate, info[1]);
    vfs::Bitfield<vfs::OpenFlags> flags;
    bool fR = false, fW= false;
    for (char p : strFlags)
    {
        switch (p)
        {
        case 'r': fR = true; break;
        case 'w': fW = true; break;
        case '+': flags |= vfs::OpenFlags::kCreate; break;
        case 'a': flags |= vfs::OpenFlags::kAppend; break;
        case 't': flags |= vfs::OpenFlags::kTrunc; break;
        default: CHECK_AND_JS_THROW_WITH_RET(true, "Bad open mode", -1);
        }
    }
    if (fR && fW)
        flags |= vfs::OpenFlags::kReadWrite;
    else if (fR)
        flags |= vfs::OpenFlags::kReadonly;
    else if (fW)
        flags |= vfs::OpenFlags::kWriteOnly;

    vfs::Bitfield<vfs::Mode> mode({vfs::Mode::kUsrR, vfs::Mode::kUsrW,
                                   vfs::Mode::kGrpR, vfs::Mode::kGrpW,
                                   vfs::Mode::kOthR});
    if (info.Length() == 3)
    {
        CHECK_AND_JS_THROW_WITH_RET(!info[2]->IsUint32(),
                                    "Creation mode should be an unsigned integer", -1);
        mode = vfs::Bitfield<vfs::Mode>(binder::from_v8<uint32_t>(isolate, info[2]));
    }
    int32_t fd = vfs::Open(path, flags, mode);
    return fd < 0 ? fd : FDLRNewRandomizedDescriptor(fd, FDLRTable::OwnerType::kUser, [](int32_t fd) {
        vfs::Close(fd);
    });
}

void jni_core_close(int32_t fd)
{
    FDLRTable::Target *target = FDLRGetUnderlyingDescriptor(fd);
    CHECK_AND_JS_THROW(!target, "Corrupted file descriptor");
    CHECK_AND_JS_THROW(!target->closer, "File descriptor is not closable");
    target->closer(target->fd);
    FDLRMarkUnused(fd);
}

void jni_core_dump(const std::string& what)
{
    if (what == "descriptors-info")
    {
        FDLRDumpMappingInfo();
    }
    else
    {
        CHECK_AND_JS_THROW(false, "Unknown dump target");
    }
}

void jni_core_exit()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->TerminateExecution();
    EventLoop::Instance()->dispose();
    LOGW(LOG_DEBUG, "Program will be terminated directly by JavaScript")
}

v8::Local<v8::Array> GetEscapableArgs()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Array> array = v8::Array::New(isolate);

    auto args = prop::Cast<PropertyArrayNode>(prop::Get()->next("runtime")->next("script")->next("args"));
    uint32_t index = 0;
    for (const std::shared_ptr<PropertyNode>& node : *args)
    {
        const std::string& str = prop::Cast<PropertyDataNode>(node)->extract<std::string>();
        array->Set(isolate->GetCurrentContext(), index, binder::to_v8(isolate, str)).Check();
        index++;
    }
    return scope.Escape(array);
}

CoreBinding::CoreBinding()
        : BindingBase("core",
                      "Basic language features for Cocoa JavaScript")
{
    FDLRInitialize();
}

CoreBinding::~CoreBinding()
{
    FDLRCollectAndSweep();
}

void CoreBinding::setInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    instance->Set(context, binder::to_v8(isolate, "args"), GetEscapableArgs()).Check();
}

KOI_LANG_NS_END
