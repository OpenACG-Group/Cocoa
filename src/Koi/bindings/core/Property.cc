#include <optional>

#include "include/v8.h"
#include "fmt/format.h"

#include "Core/Properties.h"
#include "Koi/bindings/core/Exports.h"

KOI_BINDINGS_NS_BEGIN

std::optional<int> parsePropertyArraySubscript(const std::string_view& spec,
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
        binder::throw_(isolate, "Array subscript should be an integer", v8::Exception::Error);
        thrown = true;
        return {};
    }

    if (subscript_val < 0)
    {
        binder::throw_(isolate, "Array subscript should be a positive integer", v8::Exception::Error);
        thrown = true;
        return {};
    }

    if (subscript_val > INT_MAX)
    {
        binder::throw_(isolate, "Array subscript is too large", v8::Exception::Error);
        thrown = true;
        return {};
    }

    return std::make_optional(static_cast<int>(subscript_val));
}

std::shared_ptr<PropertyNode> parsePropertySpec(const std::string& spec,
                                                bool& thrown,
                                                v8::Isolate *isolate)
{
    thrown = false;

    std::vector<std::string_view> selectors;
    std::size_t p = 0;
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

        auto maybe_array = parsePropertyArraySubscript(sel, thrown, isolate);
        if (thrown)
            return nullptr;

        if (maybe_array)
        {
            if (currentNode->kind() != PropertyNode::Kind::kArray)
            {
                binder::throw_(isolate, "Illegal usage of array subscript", v8::Exception::Error);
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
                binder::throw_(isolate, "Illegal usage of member selector", v8::Exception::Error);
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

namespace
{
std::map<PropertyNode::Kind, const char*> gNodeKindNameMap = {
        {PropertyNode::Kind::kData, "data"},
        {PropertyNode::Kind::kObject, "object"},
        {PropertyNode::Kind::kArray, "array"}
};
} // namespace anonymous

v8::Local<v8::Value> coreGetProperty(const std::string& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    bool thrown = false;
    std::shared_ptr<PropertyNode> maybeNode = parsePropertySpec(spec, thrown, isolate);
    if (thrown)
        return {};

    if (!maybeNode || maybeNode->protection() == PropertyNode::Protection::kPrivate)
    {
        binder::throw_(isolate, "No such property or inaccessible to JavaScript", v8::Exception::Error);
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

v8::Local<v8::Value> coreEnumeratePropertyNode(const std::string& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope handleScope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    bool thrown = false;
    std::shared_ptr<PropertyNode> node = parsePropertySpec(spec, thrown, isolate);
    if (thrown)
        return {};

    if (!node || node->protection() == PropertyNode::Protection::kPrivate)
    {
        binder::throw_(isolate, "No such property or inaccessible to JavaScript", v8::Exception::Error);
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

bool coreHasProperty(const std::string& spec)
{
    bool thrown = false;
    std::shared_ptr<PropertyNode> node = parsePropertySpec(spec, thrown, v8::Isolate::GetCurrent());
    if (thrown)
        return {};

    return (node && node->protection() != PropertyNode::Protection::kPrivate);
}

KOI_BINDINGS_NS_END
