#include <istream>
#include <fstream>

#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>

#include "Core/StrictJSONParser.h"

namespace cocoa::json {

namespace {
std::map<AnyTemplate::Kind, const char*> templateKindName = {
        { AnyTemplate::Kind::kObjectTemplate, "object" },
        { AnyTemplate::Kind::kArrayTemplate,   "array" },
        { AnyTemplate::Kind::kFinalValueTemplate, "final-value" }
};

std::map<FinalValueTemplate::Type, const char*> finalValueTypeName = {
        { FinalValueTemplate::kType_Boolean, "boolean" },
        { FinalValueTemplate::kType_String, "string" },
        { FinalValueTemplate::kType_Float, "float" },
        { FinalValueTemplate::kType_Integer, "integer" }
};

bool matchTemplateType(AnyTemplate::Kind kind, const std::type_info& type)
{
    if (type == typeid(Poco::JSON::Object::Ptr))
        return kind == AnyTemplate::Kind::kObjectTemplate;
    else if (type == typeid(Poco::JSON::Array::Ptr))
        return kind == AnyTemplate::Kind::kArrayTemplate;
    else
        return kind == AnyTemplate::Kind::kFinalValueTemplate;
}

bool matchFinalValueType(FinalValueTemplate::Type templateType, const std::type_info& type)
{
    if (type == typeid(long))
        return templateType == FinalValueTemplate::kType_Integer;
    else if (type == typeid(double))
        return templateType == FinalValueTemplate::kType_Float;
    else if (type == typeid(std::string))
        return templateType == FinalValueTemplate::kType_String;
    else if (type == typeid(bool))
        return templateType == FinalValueTemplate::kType_Boolean;
    return false;
}

template<typename T>
T *CreateOrReuseNode(PropertyTreeNode *pParentNode, const std::string& name)
{
    return nullptr;
}

PropertyTreeNode *findNodeByName(PropertyTreeNode *pParentNode, const std::string& name)
{
    for (auto *pNode : pParentNode->children())
    {
        if (pNode->name() == name)
            return pNode;
    }

    return nullptr;
}

template<>
PropertyTreeDirNode *CreateOrReuseNode(PropertyTreeNode *pParentNode, const std::string& name)
{
    auto *pNode = findNodeByName(pParentNode, name);
    if (pNode != nullptr)
        return pNode->cast<PropertyTreeDirNode>();

    if (name == "<anonymous>")
        return PropertyTreeNode::NewDirNode(pParentNode)->cast<PropertyTreeDirNode>();
    return PropertyTreeNode::NewDirNode(pParentNode, name)->cast<PropertyTreeDirNode>();
}

template<>
PropertyTreeArrayNode *CreateOrReuseNode(PropertyTreeNode *pParentNode, const std::string& name)
{
    /* For array node, it can't be reused */
    /* We don't need any 'if', it's safe to delete a null pointer  */
    delete findNodeByName(pParentNode, name);

    if (name == "<anonymous>")
        return PropertyTreeNode::NewArrayNode(pParentNode)->cast<PropertyTreeArrayNode>();
    return PropertyTreeNode::NewArrayNode(pParentNode, name)->cast<PropertyTreeArrayNode>();
}

void parseJSONValue(const std::string& name,
                    const Poco::Dynamic::Var& value,
                    const FinalValueTemplate *pTemplate,
                    PropertyTreeNode *pParentNode);

void parseJSONArray(Poco::JSON::Array::Ptr pArray, const ArrayTemplate *pTemplate,
                    PropertyTreeArrayNode *pNode);

void parseJSONObject(Poco::JSON::Object::Ptr pObject, const ObjectTemplate *pTemplate,
                     PropertyTreeDirNode *pNode);

void selectFitJSONParser(const Poco::Dynamic::Var& value,
                         const AnyTemplate *pTemplate,
                         PropertyTreeNode *pParentNode)
{
    switch (pTemplate->kind())
    {
    case AnyTemplate::Kind::kObjectTemplate:
        parseJSONObject(value.extract<Poco::JSON::Object::Ptr>(),
                        pTemplate->cast<ObjectTemplate>(),
                        CreateOrReuseNode<PropertyTreeDirNode>(pParentNode, pTemplate->name()));
        break;
    case AnyTemplate::Kind::kArrayTemplate:
        parseJSONArray(value.extract<Poco::JSON::Array::Ptr>(),
                       pTemplate->cast<ArrayTemplate>(),
                       CreateOrReuseNode<PropertyTreeArrayNode>(pParentNode, pTemplate->name()));
        break;
    case AnyTemplate::Kind::kFinalValueTemplate:
        parseJSONValue(pTemplate->name(), value,
                       pTemplate->cast<FinalValueTemplate>(), pParentNode);
        break;
    }
}

void parseJSONValue(const std::string& name,
                    const Poco::Dynamic::Var& value,
                    const FinalValueTemplate *pTemplate,
                    PropertyTreeNode *pParentNode)
{
    if (!matchFinalValueType(pTemplate->type(), value.type()))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("JSON field ").append(name)
                .append(" should be ").append(finalValueTypeName[pTemplate->type()])
                .append(" type")
                .make<RuntimeException>();
    }

    if (name == "<anonymous>")
        PropertyTreeNode::NewDataNode(pParentNode, value);
    else
    {
        delete findNodeByName(pParentNode, name);
        PropertyTreeNode::NewDataNode(pParentNode, name, value);
    }
}

void parseJSONArray(Poco::JSON::Array::Ptr pArray, const ArrayTemplate *pTemplate,
                    PropertyTreeArrayNode *pNode)
{
    const AnyTemplate *elementTemplate = pTemplate->elementTemplate();

    for (const auto& element : *pArray)
    {
        if (!matchTemplateType(elementTemplate->kind(), element.type()))
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("Elements in JSON array have a bad type")
                    .make<RuntimeException>();
        }

        selectFitJSONParser(element, elementTemplate, pNode);
    }
}

void parseJSONObject(Poco::JSON::Object::Ptr pObject, const ObjectTemplate *pTemplate,
                     PropertyTreeDirNode *pNode)
{
    /* Creates a use table for each required member in template */
    std::map<const char*, bool> useTable;
    for (const AnyTemplate *pMember : pTemplate->members())
    {
        if (!pMember->optional())
            useTable[pMember->name()] = false;
    }

    for (const auto& member : *pObject)
    {
        /* Match by member's name */
        const AnyTemplate *pMemberTemplate = nullptr;
        for (const AnyTemplate *pTemplateMember : pTemplate->members())
        {
            if (member.first == pTemplateMember->name())
            {
                pMemberTemplate = pTemplateMember;
                break;
            }
        }
        if (pMemberTemplate == nullptr)
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("JSON template ")
                    .append(pTemplate->name())
                    .append(" not contains member named ")
                    .append(member.first)
                    .make<RuntimeException>();
        }

        /* Match by member's type */
        if (!matchTemplateType(pMemberTemplate->kind(), member.second.type()))
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("JSON field ")
                    .append(member.first)
                    .append(" should be a(n) ")
                    .append(templateKindName[pMemberTemplate->kind()])
                    .make<RuntimeException>();
        }

        if (!pMemberTemplate->optional())
            useTable[pMemberTemplate->name()] = true;

        selectFitJSONParser(member.second, pMemberTemplate, pNode);
    }

    for (const auto& p : useTable)
    {
        if (!p.second)
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("JSON field ")
                    .append(p.first)
                    .append(" must be specified explicitly")
                    .make<RuntimeException>();
        }
    }
}

void parseJSON(const Poco::Dynamic::Var& json, const ObjectTemplate *pTemplate, PropertyTreeNode *pNode)
{
    if (json.type() != typeid(Poco::JSON::Object::Ptr))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("JSON should be rooted in object")
                .make<RuntimeException>();
    }

    parseJSONObject(json.extract<Poco::JSON::Object::Ptr>(),
                    pTemplate, pNode->cast<PropertyTreeDirNode>());
}

} // namespace anonymous

void parseFile(const std::string& file, const ObjectTemplate *pTemplate, PropertyTreeNode *pNode)
{
    std::ifstream fs(file);
    if (!fs.is_open())
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Couldn\'t connectDisplay JSON file ")
                .append(file)
                .make<RuntimeException>();
    }

    try {
        Poco::JSON::Parser parser;
        parseJSON(parser.parse(fs), pTemplate, pNode);
    } catch (const Poco::JSON::JSONException& e) {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("JSON: ")
                .append(e.displayText())
                .make<RuntimeException>();
    }
}

void parseString(const std::string& content, const ObjectTemplate *pTemplate, PropertyTreeNode *pNode)
{
    try {
        Poco::JSON::Parser parser;
        parseJSON(parser.parse(content), pTemplate, pNode);
    } catch (const Poco::JSON::JSONException& e) {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("JSON: ")
                .append(e.displayText())
                .make<RuntimeException>();
    }
}

AnyTemplate::AnyTemplate(Kind kind, bool optional, const char *name) noexcept
    : fKind(kind), fOptional(optional), fName(name) {}

ObjectTemplate::ObjectTemplate(bool optional,
               const char *name,
               const std::initializer_list<const AnyTemplate*>& members) noexcept
        : AnyTemplate(Kind::kObjectTemplate, optional, name),
          fMembers(members) {}

ObjectTemplate::ObjectTemplate(bool optional,
               const std::initializer_list<const AnyTemplate*>& members) noexcept
        : AnyTemplate(Kind::kObjectTemplate, optional, "<anonymous>"),
          fMembers(members) {}

ArrayTemplate::ArrayTemplate(bool optional, const char *name, AnyTemplate *elementTemp) noexcept
        : AnyTemplate(Kind::kArrayTemplate, optional, name),
          fElementTemp(elementTemp) {}

ArrayTemplate::ArrayTemplate(bool optional, AnyTemplate *elementTemp) noexcept
        : AnyTemplate(Kind::kArrayTemplate, optional, "<anonymous>"),
          fElementTemp(elementTemp) {}

FinalValueTemplate::FinalValueTemplate(bool optional, char const *name, Type type) noexcept
        : AnyTemplate(Kind::kFinalValueTemplate, optional, name),
          fType(type) {}

FinalValueTemplate::FinalValueTemplate(bool optional, Type type) noexcept
        : AnyTemplate(Kind::kFinalValueTemplate, optional, "<anonymous>"),
          fType(type) {}

} // namespace cocoa::json

