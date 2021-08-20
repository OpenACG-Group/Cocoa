#include <memory>
#include <string>
#include <vector>
#include <map>
#include <any>

#include "Core/Exception.h"
#include "Core/Properties.h"
namespace cocoa {

PropertyNode::PropertyNode(Kind kind)
    : fKind(kind),
      fProtection(Protection::kDefault)
{
}

void PropertyNode::setParent(const std::shared_ptr<PropertyNode>& node)
{
    fProtection = node->protection();
    fParent = node;
}

std::shared_ptr<PropertyNode> PropertyNode::next(uint32_t i0)
{
    if (fKind != Kind::kArray)
        throw RuntimeException(__func__, "Property node is not an array");

    auto ptr = prop::Cast<PropertyArrayNode>(shared_from_this())->at(i0);
    if (!ptr)
        throw RuntimeException(__func__, "Illegal index");
    return ptr;
}

std::shared_ptr<PropertyNode> PropertyNode::next(const std::string& member)
{
    if (fKind != Kind::kObject)
        throw RuntimeException(__func__, "Property node is not an object");

    auto ptr = prop::Cast<PropertyObjectNode>(shared_from_this())->getMember(member);
    if (!ptr)
        throw RuntimeException(__func__, "Illegal name of member");
    return ptr;
}

PropertyObjectNode::PropertyObjectNode()
    : PropertyNode(Kind::kObject)
{
}

std::shared_ptr<PropertyNode> PropertyObjectNode::getMember(const std::string& name)
{
    if (!fMembers.contains(name))
        return nullptr;
    return fMembers[name];
}

void PropertyObjectNode::unsetMember(const std::string& name)
{
    if (!fMembers.contains(name))
        return;
    fMembers.erase(name);
}

void PropertyObjectNode::setMember(const std::string& name,
                                   std::shared_ptr<PropertyNode> member)
{
    member->setParent(shared_from_this());
    fMembers[name] = std::move(member);
}

bool PropertyObjectNode::hasMember(const std::string& name)
{
    return fMembers.contains(name);
}

PropertyArrayNode::PropertyArrayNode()
    : PropertyNode(Kind::kArray)
{
}

void PropertyArrayNode::append(std::shared_ptr<PropertyNode> node)
{
    node->setParent(shared_from_this());
    fSubscripts.emplace_back(std::move(node));
}

std::shared_ptr<PropertyNode> PropertyArrayNode::at(uint32_t i0)
{
    if (i0 >= fSubscripts.size())
        return nullptr;
    return fSubscripts[i0];
}

void PropertyArrayNode::erase(uint32_t i0)
{
    if (i0 >= fSubscripts.size())
        return;
    fSubscripts.erase(fSubscripts.begin() + i0);
}

PropertyDataNode::PropertyDataNode(std::any&& value)
    : PropertyNode(Kind::kData),
      fData(value)
{
}

void PropertyDataNode::reset(std::any&& value)
{
    fData = value;
}

const std::type_info& PropertyDataNode::type()
{
    return fData.type();
}

namespace {
std::shared_ptr<PropertyNode> gPropRoot;

[[maybe_unused]] __attribute__((constructor)) void _initializer()
{
    gPropRoot = std::make_shared<PropertyObjectNode>();
}

[[maybe_unused]] __attribute__((destructor)) void _finalizer()
{
    gPropRoot = nullptr;
}


} // namespace anonymous

namespace prop
{

std::shared_ptr<PropertyObjectNode> Get()
{
    return Cast<PropertyObjectNode>(gPropRoot);
}

} // namespace properties
} // namespace cocoa
