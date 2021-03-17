#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <Poco/Dynamic/Var.h>

#include "Core/Exception.h"
#include "Core/PropertyTree.h"

namespace {

std::vector<std::string> parsePropertyPath(const std::string& path)
{
    std::string buffer;
    std::vector<std::string> ret;

    for (char ch : path)
    {
        if (ch == '/')
        {
            if (!buffer.empty())
            {
                ret.push_back(buffer);
                buffer.clear();
            }
        }
        else
            buffer.push_back(ch);
    }
    if (!buffer.empty())
        ret.push_back(buffer);

    return ret;
}

} // namespace anonymous

namespace cocoa {

PropertyTreeNode::Iterable::Iterable(std::list<PropertyTreeNode *>::iterator begin,
                                     std::list<PropertyTreeNode *>::iterator end)
    : fBegin(begin),
      fEnd(end)
{
}

std::list<PropertyTreeNode *>::iterator PropertyTreeNode::Iterable::begin()
{
    return fBegin;
}

std::list<PropertyTreeNode *>::iterator PropertyTreeNode::Iterable::end()
{
    return fEnd;
}

// -------------------------------------------------------------------

static std::string NameAnonymousNode(PropertyTreeNode *parent)
{
    if (parent == nullptr ||
        parent->kind() != PropertyTreeNode::Kind::kArray)
        return "<anonymous>";

    int count = std::distance(parent->children().begin(), parent->children().end());

    std::ostringstream oss;
    oss << "<array#" << count << ">";
    return oss.str();
}

PropertyTreeNode *PropertyTreeNode::NewDirNode(PropertyTreeNode *parent, const std::string& name)
{
    auto *ptr = new PropertyTreeDirNode(parent, name);
    if (parent)
        parent->appendChild(ptr);
    return ptr;
}

PropertyTreeNode *PropertyTreeNode::NewDirNode(PropertyTreeNode *parent)
{
    auto *ptr = new PropertyTreeDirNode(parent, NameAnonymousNode(parent));
    if (parent)
        parent->appendChild(ptr);
    return ptr;
}

PropertyTreeNode *PropertyTreeNode::NewArrayNode(PropertyTreeNode *parent, const std::string& name)
{
    auto *ptr = new PropertyTreeArrayNode(parent, name);
    if (parent)
        parent->appendChild(ptr);
    return ptr;
}

PropertyTreeNode *PropertyTreeNode::NewArrayNode(PropertyTreeNode *parent)
{
    auto *ptr = new PropertyTreeArrayNode(parent, NameAnonymousNode(parent));
    if (parent)
        parent->appendChild(ptr);
    return ptr;
}

PropertyTreeNode * PropertyTreeNode::NewDataNode(PropertyTreeNode *parent, const std::string& name,
                                                 const Poco::Dynamic::Var& val)
{
    auto *ptr = new PropertyTreeDataNode(parent, name, val);
    if (parent)
        parent->appendChild(ptr);
    return ptr;
}

PropertyTreeNode *PropertyTreeNode::NewDataNode(PropertyTreeNode *parent, const Poco::Dynamic::Var& val)
{
    auto *ptr = new PropertyTreeDataNode(parent, NameAnonymousNode(parent), val);
    if (parent)
        parent->appendChild(ptr);
    return ptr;
}

// ------------------------------------------------------------------------------------

PropertyTreeNode::PropertyTreeNode(PropertyTreeNode *parent, Kind kind, const std::string& name)
    : fParent(parent),
      fKind(kind),
      fName(name)
{}

PropertyTreeNode::~PropertyTreeNode()
{
    if (fParent)
        fParent->removeChild(this);

    std::list<PropertyTreeNode*> children = fChildren;
    for (PropertyTreeNode *pNode : children)
        delete pNode;
}

void PropertyTreeNode::appendChild(PropertyTreeNode *child)
{
    fChildren.push_back(child);
}

void PropertyTreeNode::removeChild(PropertyTreeNode *child)
{
    fChildren.remove_if([&](PropertyTreeNode *pNode) -> bool {
        return pNode == child;
    });
}

std::string PropertyTreeNode::name()
{
    return fName;
}

PropertyTreeNode::Kind PropertyTreeNode::kind()
{
    return fKind;
}

PropertyTreeNode::Iterable PropertyTreeNode::children()
{
    return Iterable(fChildren.begin(), fChildren.end());
}

// -----------------------------------------------------------------------------

PropertyTreeDirNode::PropertyTreeDirNode(PropertyTreeNode *parent, const std::string& name)
    : PropertyTreeNode(parent, Kind::kDir, name)
{
}


PropertyTreeArrayNode::PropertyTreeArrayNode(PropertyTreeNode *parent, const std::string& name)
    : PropertyTreeNode(parent, Kind::kArray, name)
{
}

Poco::Dynamic::Var& PropertyTreeDataNode::value()
{
    return fData;
}

// ------------------------------------------------------------------------------

PropertyTree::PropertyTree()
{
    fRoot = new PropertyTreeDirNode(nullptr, "<root>");
}

PropertyTree::~PropertyTree()
{
    delete fRoot;
}

PropertyTreeNode *PropertyTree::resolve(const std::string& path)
{
    PropertyTreeNode *pNode = fRoot;
    auto directive = parsePropertyPath(path);

    for (const std::string& str : directive)
    {
        bool found = false;
        for (auto *pChild : pNode->children())
        {
            if (pChild->name() == str)
            {
                pNode = pChild;
                found = true;
                break;
            }
        }

        if (!found)
            return nullptr;
    }
    return pNode;
}

}
