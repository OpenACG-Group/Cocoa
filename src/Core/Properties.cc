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

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <any>
#include <stack>

#include "Core/Exception.h"
#include "Core/Properties.h"
#include "Core/Errors.h"
#include "Core/Utils.h"
namespace cocoa {

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Core.Property)

std::string protection_to_string(PropertyNode::Protection prot)
{
    switch (prot)
    {
    case PropertyNode::Protection::kPublic:
        return "public";
    case PropertyNode::Protection::kPrivate:
        return "private";
    }
    MARK_UNREACHABLE();
}

PropertyNode::PropertyNode(Kind kind)
    : fKind(kind),
      fProtection(Protection::kDefault)
{
}

void PropertyNode::setParent(const std::shared_ptr<PropertyNode>& node)
{
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

std::string PropertyNode::getName()
{
    if (fParent.expired())
        return "<orphan>";
    auto parent = this->parent();
    CHECK(parent->kind() != Kind::kData);
    if (parent->kind() == Kind::kArray)
    {
        int32_t idx = 0;
        for (const auto& p : *prop::Cast<PropertyArrayNode>(parent))
        {
            if (p.get() == this)
                return fmt::format("[{}]", idx);
            idx++;
        }
        return "<unnamed>";
    }
    else if (parent->kind() == Kind::kObject)
    {
        for (const auto& p : *prop::Cast<PropertyObjectNode>(parent))
        {
            if (p.second.get() == this)
                return p.first;
        }
        return "<unknown>";
    }
    MARK_UNREACHABLE();
}

PropertyObjectNode::PropertyObjectNode()
    : PropertyNode(Kind::kObject)
{
}

std::string PropertyObjectNode::toQLogString()
{
    return fmt::format("%fg<gr,hl>ObjectNode%reset %fg<ye>{}%reset %italic%fg<re><{}>%reset %fg<gr,hl>{}%reset",
                       fmt::ptr(this), protection_to_string(protection()), getName());
}

std::shared_ptr<PropertyNode> PropertyObjectNode::getMember(const std::string& name)
{
    if (!utils::MapContains(fMembers, name))
        return nullptr;
    return fMembers[name];
}

void PropertyObjectNode::unsetMember(const std::string& name)
{
    if (!utils::MapContains(fMembers, name))
        return;
    fMembers.erase(name);
}

std::shared_ptr<PropertyNode> PropertyObjectNode::setMember(const std::string& name,
                                                            const std::shared_ptr<PropertyNode>& member)
{
    member->setParent(shared_from_this());
    fMembers[name] = member;

    return member;
}

void PropertyObjectNode::renameMember(const std::string& oldName, const std::string& newName)
{
    if (fMembers.count(oldName) > 0 && fMembers.count(newName) == 0)
    {
        fMembers[newName] = fMembers[oldName];
        fMembers.erase(oldName);
    }
}

bool PropertyObjectNode::hasMember(const std::string& name)
{
    return utils::MapContains(fMembers, name);
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

std::string PropertyArrayNode::toQLogString()
{
    return fmt::format(
            "%fg<ma,hl>ArrayNode%reset %fg<ye>{}%reset %italic%fg<re><{}, size={}>%reset %fg<gr,hl>{}%reset",
            fmt::ptr(this), protection_to_string(protection()), fSubscripts.size(), getName());
}

PropertyDataNode::PropertyDataNode(std::any&& value)
    : PropertyNode(Kind::kData),
      fData(value)
{
}

PropertyDataNode::PropertyDataNode()
    : PropertyNode(Kind::kData)
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

std::string PropertyDataNode::toQLogString()
{
    return fmt::format("%fg<cy,hl>DataNode%reset %fg<ye>{}%reset %italic%fg<re><{}>%reset %fg<gr,hl>{}%reset",
                       fmt::ptr(this), protection_to_string(protection()), getName());
}

namespace {
std::shared_ptr<PropertyNode> gPropRoot;

[[maybe_unused]] __attribute__((constructor)) void _initializer()
{
    gPropRoot = std::make_shared<PropertyObjectNode>();
    gPropRoot->setProtection(PropertyNode::Protection::kPublic);
}

[[maybe_unused]] __attribute__((destructor)) void _finalizer()
{
    gPropRoot = nullptr;
}

void serializeNode(const std::shared_ptr<PropertyNode>& node, std::string prefix)
{
    QLOG(LOG_DEBUG, "{}{}", prefix, node->toQLogString());
    if (!prefix.empty())
    {
        prefix[prefix.size() - 1] = ' ';
        if (prefix[prefix.size() - 2] == '`')
            prefix[prefix.size() - 2] = ' ';
    }
    std::string np = prefix + "|-";
    std::string np2 = prefix + "`-";
    node->forEachChild([&np, &np2](const std::shared_ptr<PropertyNode>& child, bool last) {
        serializeNode(child, last ? np2 : np);
    });
}

} // namespace anonymous

namespace prop
{

std::shared_ptr<PropertyObjectNode> Get()
{
    return Cast<PropertyObjectNode>(gPropRoot);
}

void SerializeToJournal(const std::shared_ptr<PropertyObjectNode>& root)
{
    QLOG(LOG_DEBUG, "Current properties tree:");
    serializeNode(root, "");
}

} // namespace properties
} // namespace cocoa
