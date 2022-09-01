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

#include <optional>
#include <unordered_map>
#include <map>
#include <cxxabi.h>

#include "include/v8.h"
#include "fmt/format.h"

#include "Core/Properties.h"
#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Gallium/binder/CallV8.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.core)

namespace {
std::unordered_map<std::shared_ptr<PropertyNode>, v8::Global<v8::Object>> property_cache_;

bool checkIsLegalNodeName(const std::string& name)
{
    if (name.empty())
        return false;

    if (!std::isalpha(name[0]) && name[0] != '_')
        return false;

    return std::all_of(name.begin(), name.end(), [](char c) -> bool {
        return (std::isdigit(c) || std::isalpha(c) || c == '_');
    });
}

int32_t property_direct_invocation_test(int32_t a, int32_t b)
{
    return (a + b);
}

} // namespace anonymous

void PropertyWrap::InstallProperties()
{
    auto script = prop::Get()->next("Runtime")->next("Script")->as<PropertyObjectNode>();

    script->setMember("DirectCallTestFunc",
                      prop::New<PropertyDataNode>(property_direct_invocation_test));
}

v8::Local<v8::Object> PropertyWrap::GetWrap(v8::Isolate *isolate,
                                            const std::shared_ptr<PropertyNode>& node)
{
    v8::EscapableHandleScope scope(isolate);
    if (property_cache_.count(node) > 0)
        return scope.Escape(property_cache_[node].Get(isolate));
    v8::Local<v8::Object> wrap = binder::Class<PropertyWrap>::create_object(isolate,
                                                                            node);
    v8::Global<v8::Object> global = v8::Global<v8::Object>(isolate, wrap);
    /**
     * turn this global handle to a phantom weak handle without a finalizer callback,
     * which makes sure that the wrap object can be destructed when GC happens.
     */
    global.SetWeak();

    property_cache_[node] = std::move(global);
    return scope.Escape(wrap);
}

PropertyWrap::PropertyWrap(const std::shared_ptr<PropertyNode>& node)
    : fNode(node)
{
    CHECK(node);
}

PropertyWrap::~PropertyWrap()
{
    std::shared_ptr<PropertyNode> node = fNode.lock();
    if (property_cache_.count(node) > 0)
    {
        property_cache_[node].Reset();
        property_cache_.erase(node);
    }
}

void PropertyWrap::checkNodeProtectionForJSWriting() const
{
    if (lockNode()->protection() != PropertyNode::Protection::kPublic)
    {
        g_throw(Error, "Permission denied for property accessing");
    }
}

v8::Local<v8::Value> PropertyWrap::getType() const
{
    auto node = lockNode();
    Type tp;
    switch (node->kind())
    {
    case PropertyNode::Kind::kObject:
        tp = Type::kObject;
        break;
    case PropertyNode::Kind::kArray:
        tp = Type::kArray;
        break;
    case PropertyNode::Kind::kData:
        tp = Type::kData;
        break;
    }

    return binder::to_v8(v8::Isolate::GetCurrent(), static_cast<uint32_t>(tp));
}

v8::Local<v8::Value> PropertyWrap::getParent() const
{
    auto node = lockNode();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!node->parent())
        return v8::Null(isolate);

    return GetWrap(isolate, node->parent());
}

v8::Local<v8::Value> PropertyWrap::getName() const
{
    auto node = lockNode();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    return binder::to_v8(isolate, node->getName());
}

void PropertyWrap::setName(v8::Local<v8::Value> name) const
{
    checkNodeProtectionForJSWriting();
    if (!name->IsString())
        g_throw(TypeError, "name must be a string");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::string newName = binder::from_v8<std::string>(isolate, name);
    if (!checkIsLegalNodeName(newName))
        g_throw(Error, "Illegal node name");

    auto node = lockNode();
    if (!node->parent())
        g_throw(Error, "Cannot set name for orphan node");
    if (node->parent()->kind() != PropertyNode::Kind::kObject)
        g_throw(Error, "Cannot set name for node which has non-object parent");

    auto parent = node->parent()->as<PropertyObjectNode>();
    if (parent->hasMember(newName))
        g_throw(Error, "Name has already been used");

    parent->renameMember(node->getName(), newName);
}

v8::Local<v8::Value> PropertyWrap::getProtection() const
{
    Prot prot;
    switch (lockNode()->protection())
    {
    case PropertyNode::Protection::kPublic:
        prot = Prot::kPublic;
        break;
    case PropertyNode::Protection::kPrivate:
        prot = Prot::kPrivate;
        break;
    }
    return binder::to_v8(v8::Isolate::GetCurrent(), static_cast<uint32_t>(prot));
}

v8::Local<v8::Value> PropertyWrap::getNumberOfChildren() const
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    PropertyNode::Kind kind = lockNode()->kind();
    if (kind == PropertyNode::Kind::kObject)
    {
        auto node = lockNode()->as<PropertyObjectNode>();
        return binder::to_v8(isolate, std::distance(node->begin(), node->end()));
    }
    else if (kind == PropertyNode::Kind::kArray)
    {
        auto array = lockNode()->as<PropertyArrayNode>();
        return binder::to_v8(isolate, array->size());
    }
    else
        return binder::to_v8(isolate, 0);
}

namespace {

bool invokeForeachCallback(const std::shared_ptr<PropertyNode>& child,
                           v8::Local<v8::Value> callback)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    v8::Local<v8::Object> wrap = PropertyWrap::GetWrap(isolate, child);

    v8::TryCatch tryCatch(isolate);
    binder::Invoke(isolate, callback.As<v8::Function>(),
                   isolate->GetCurrentContext()->Global(), wrap);
    if (tryCatch.HasCaught())
    {
        tryCatch.ReThrow();
        return false;
    }
    return true;
}

} // namespace anonymous

void PropertyWrap::foreachChild(v8::Local<v8::Value> callback) const
{
    if (!callback->IsFunction())
        g_throw(TypeError, "Callback must be a function");
    PropertyNode::Kind kind = lockNode()->kind();

    if (kind == PropertyNode::Kind::kObject)
    {
        auto node = lockNode()->as<PropertyObjectNode>();
        for (const auto& pair : *node)
        {
            if (!invokeForeachCallback(pair.second, callback))
                return;
        }
    }
    else if (kind == PropertyNode::Kind::kArray)
    {
        auto node = lockNode()->as<PropertyArrayNode>();
        for (const auto& child : *node)
        {
            if (!invokeForeachCallback(child, callback))
                return;
        }
    }
}

void PropertyWrap::detachFromParent()
{
    checkNodeProtectionForJSWriting();
    auto node = lockNode();
    if (!node->parent())
        g_throw(Error, "Detach an orphan node");

    auto parent = node->parent();

    if (parent->kind() == PropertyNode::Kind::kArray)
    {
        auto array = parent->as<PropertyArrayNode>();
        for (size_t i = 0; i < array->size(); i++)
        {
            if (array->at(i) == node)
            {
                array->erase(i);
                break;
            }
        }
    }
    else if (parent->kind() == PropertyNode::Kind::kObject)
    {
        parent->as<PropertyObjectNode>()->unsetMember(node->getName());
    }
    else
    {
        MARK_UNREACHABLE();
    }
}

namespace {

std::shared_ptr<PropertyNode> createPropertyNode(v8::Isolate *isolate, int32_t type)
{
    std::shared_ptr<PropertyNode> child;

    switch (type)
    {
    case static_cast<uint32_t>(PropertyWrap::Type::kObject):
        child = prop::New<PropertyObjectNode>();
        break;
    case static_cast<uint32_t>(PropertyWrap::Type::kArray):
        child = prop::New<PropertyArrayNode>();
        break;
    case static_cast<uint32_t>(PropertyWrap::Type::kData):
        child = prop::New<PropertyDataNode>();
        break;
    default:
        g_throw3(RangeError, "Invalid type", isolate);
    }

    child->setProtection(PropertyNode::Protection::kPublic);

    return child;
}

} // namespace anonymous

v8::Local<v8::Value> PropertyWrap::insertChild(int32_t type, const std::string& name) const
{
    checkNodeProtectionForJSWriting();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (!checkIsLegalNodeName(name))
        g_throw(Error, "Illegal node name");

    if (lockNode()->kind() != PropertyNode::Kind::kObject)
        g_throw(TypeError, "insertChild only available for object node");
    auto node = lockNode()->as<PropertyObjectNode>();

    if (node->hasMember(name))
        g_throw(Error, "Name has already been used");

    auto child = createPropertyNode(isolate, type);

    node->setMember(name, child);

    return GetWrap(isolate, child);
}

v8::Local<v8::Value> PropertyWrap::findChild(const std::string& name) const
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (lockNode()->kind() == PropertyNode::Kind::kObject)
    {
        auto node = lockNode()->as<PropertyObjectNode>()->getMember(name);
        if (node)
            return GetWrap(isolate, node);
        else
            return v8::Null(isolate);
    }
    else
        g_throw(TypeError, "findChild only available for object node");
}

v8::Local<v8::Value> PropertyWrap::pushbackChild(int32_t type) const
{
    checkNodeProtectionForJSWriting();
    if (lockNode()->kind() != PropertyNode::Kind::kArray)
    {
        g_throw(TypeError, "pushbackChild only available for array node");
    }

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto child = createPropertyNode(isolate, type);

    lockNode()->as<PropertyArrayNode>()->append(child);

    return GetWrap(isolate, child);
}

namespace {

// NOLINTNEXTLINE
bool __typeinfo_is_pointer(const std::type_info& typeinfo)
{
#if defined(__GNUG__)
    return typeinfo.__is_pointer_p();
#else
    return false;
#endif /* __GNUG__ */
}

} // namespace anonymous

std::string PropertyWrap::dataTypeinfo() const
{
    if (lockNode()->kind() != PropertyNode::Kind::kData)
    {
        g_throw(TypeError, "dataValueRTTI only available for data node");
    }

    auto node = lockNode()->as<PropertyDataNode>();

    const std::type_info& info = node->type();
    int status;
    const char *demangled = abi::__cxa_demangle(info.name(), nullptr, nullptr, &status);

    ScopeExitAutoInvoker epi([demangled] { std::free(const_cast<char*>(demangled)); });

    if (status != 0)
    {
        demangled = info.name();
        epi.cancel();
    }

    bool is_pointer_p = __typeinfo_is_pointer(info);
    // [{p}real_type]
    return fmt::format("[{{{}}}{}]", is_pointer_p ? "p" : "", demangled);
}

namespace {

#define ARGS v8::Isolate *i, const std::shared_ptr<PropertyDataNode> &n
#define RET v8::Local<v8::Value>
#define TYPE_EXTRACTOR(T, obj, creator, ...) \
    [](ARGS) -> RET { return v8::obj::creator(i, n->extract<T>()) __VA_OPT__(.) __VA_ARGS__; }

#define EXTRACTOR_ENTRY(T, obj, creator) \
    { &typeid(T), TYPE_EXTRACTOR(T, obj, creator) }

#define EXTRACTOR_MAYBE_ENTRY(T, obj, creator) \
    { &typeid(T), TYPE_EXTRACTOR(T, obj, creator, FromMaybe(v8::Null(i))) }

// NOLINTNEXTLINE
std::map<const std::type_info *, RET(*)(ARGS)> primitive_type_extractor_map_ = {
        EXTRACTOR_ENTRY(int8_t, Integer, New),
        EXTRACTOR_ENTRY(uint8_t, Integer, NewFromUnsigned),
        EXTRACTOR_ENTRY(int16_t, Integer, New),
        EXTRACTOR_ENTRY(uint16_t, Integer, NewFromUnsigned),
        EXTRACTOR_ENTRY(int32_t, Integer, New),
        EXTRACTOR_ENTRY(uint32_t, Integer, NewFromUnsigned),
        EXTRACTOR_ENTRY(int64_t, BigInt, New),
        EXTRACTOR_ENTRY(uint64_t, BigInt, NewFromUnsigned),
        EXTRACTOR_ENTRY(float, Number, New),
        EXTRACTOR_ENTRY(double, Number, New),
        EXTRACTOR_ENTRY(bool, Boolean, New),
        EXTRACTOR_MAYBE_ENTRY(const char*, String, NewFromUtf8),
        EXTRACTOR_MAYBE_ENTRY(char*, String, NewFromUtf8),
        { &typeid(std::string), [](ARGS) -> RET {
                return v8::String::NewFromUtf8(i, n->extract<std::string>().c_str())
                       .FromMaybe(v8::Null(i));
            }
        }
};

#undef EXTRACTOR_MAYBE_ENTRY
#undef EXTRACTOR_ENTRY
#undef TYPE_EXTRACTOR
#undef ARGS
#undef RET

} // namespace anonymous

v8::Local<v8::Value> PropertyWrap::extract() const
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto node = lockNode()->as<PropertyDataNode>();

    const std::type_info& info = node->type();

    for (const auto& match : primitive_type_extractor_map_)
    {
        if (*match.first == info)
            return match.second(isolate, node);
    }

    return v8::Null(v8::Isolate::GetCurrent());
}

bool PropertyWrap::hasData() const
{
    if (lockNode()->kind() != PropertyNode::Kind::kData)
    {
        g_throw(TypeError, "hasData only available for data node");
    }

    return lockNode()->as<PropertyDataNode>()->hasValue();
}

namespace {

#define RET     std::any
#define ARGS    v8::Isolate *i, v8::Local<v8::Context> c, v8::Local<v8::Value> v
#define LAMBDA  [](ARGS) -> RET
#define AS_STDCPP_STRING static_cast<std::string>(binder::from_v8<std::string>(i ,v))

// NOLINTNEXTLINE
std::unordered_map<std::string, RET(*)(ARGS)> js_primitive_converters_ = {
        { "boolean", LAMBDA { return v->ToBoolean(i)->Value(); } },
        { "number", LAMBDA { return v->ToNumber(c).ToLocalChecked()->Value(); } },
        { "bigint", LAMBDA { return AS_STDCPP_STRING; } },
        { "string", LAMBDA { return AS_STDCPP_STRING; } },
        { "symbol", LAMBDA { return AS_STDCPP_STRING; } }
};

#undef AS_STDCPP_STRING
#undef LAMBDA
#undef ARGS
#undef RET

} // namespace anonymous

void PropertyWrap::resetData(const v8::FunctionCallbackInfo<v8::Value>& args) const
{
    checkNodeProtectionForJSWriting();
    if (lockNode()->kind() != PropertyNode::Kind::kData)
        g_throw(TypeError, "resetData only available for data node");
    if (args.Length() > 1)
        g_throw(TypeError, "Too many arguments");

    auto node = lockNode()->as<PropertyDataNode>();
    if (args.Length() == 0)
    {
        node->reset({});
        return;
    }

    if (args[0]->IsNullOrUndefined())
        g_throw(TypeError, "Cannot reset value to null or undefined");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    auto type = binder::from_v8<std::string>(isolate, args[0]->TypeOf(isolate));

    if (js_primitive_converters_.count(type) > 0)
    {
        std::any value = js_primitive_converters_[type](isolate, context, args[0]);
        node->reset(std::move(value));
    }
    else
    {
        g_throw(TypeError, "Unsupported type of value");
    }
}

GALLIUM_BINDINGS_NS_END
