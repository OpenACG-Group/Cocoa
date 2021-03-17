#ifndef COCOA_PROPERTY_TREE_H
#define COCOA_PROPERTY_TREE_H

#include <string>
#include <list>
#include <Poco/Dynamic/Var.h>

#include "Core/UniquePersistent.h"
#include "Core/Exception.h"

namespace cocoa {

class PropertyTreeNode
{
public:
    enum class Kind
    {
        kDir,
        kArray,
        kData
    };

    class Iterable
    {
    public:
        Iterable(std::list<PropertyTreeNode*>::iterator begin,
                 std::list<PropertyTreeNode*>::iterator end);

        std::list<PropertyTreeNode*>::iterator begin();
        std::list<PropertyTreeNode*>::iterator end();

    private:
        std::list<PropertyTreeNode*>::iterator fBegin;
        std::list<PropertyTreeNode*>::iterator fEnd;
    };

    static PropertyTreeNode *NewDirNode(PropertyTreeNode *parent, const std::string& name);
    static PropertyTreeNode *NewDirNode(PropertyTreeNode *parent);

    static PropertyTreeNode *NewArrayNode(PropertyTreeNode *parent, const std::string& name);
    static PropertyTreeNode *NewArrayNode(PropertyTreeNode *parent);

    static PropertyTreeNode *NewDataNode(PropertyTreeNode *parent, const std::string& name,
                                         const Poco::Dynamic::Var& val);
    static PropertyTreeNode *NewDataNode(PropertyTreeNode *parent, const Poco::Dynamic::Var& val);

    PropertyTreeNode(PropertyTreeNode *parent, Kind kind, const std::string& name);
    virtual ~PropertyTreeNode();

    void appendChild(PropertyTreeNode *child);
    void removeChild(PropertyTreeNode *child);

    std::string name();
    Kind kind();
    Iterable children();

    template<typename T>
    T *cast() {
        return dynamic_cast<T*>(this);
    }

private:
    PropertyTreeNode               *fParent;
    std::list<PropertyTreeNode*>    fChildren;
    Kind                            fKind;
    std::string                     fName;
};

class PropertyTreeDirNode : public PropertyTreeNode
{
public:
    PropertyTreeDirNode(PropertyTreeNode *parent, const std::string& name);
    ~PropertyTreeDirNode() override = default;
};

/**
 * The array node is similar to directory node, but all the
 * child nodes of array node is anonymous.
 * They can only be accessed by iterator.
 */
class PropertyTreeArrayNode : public PropertyTreeNode
{
public:
    PropertyTreeArrayNode(PropertyTreeNode *parent, const std::string& name);
    ~PropertyTreeArrayNode() override = default;
};

class PropertyTreeDataNode : public PropertyTreeNode
{
public:
    template<typename T>
    PropertyTreeDataNode(PropertyTreeNode *parent, const std::string& name, T&& val)
        : PropertyTreeNode(parent, Kind::kData, name),
          fData(std::forward<T>(val)) {}
    ~PropertyTreeDataNode() override = default;

    Poco::Dynamic::Var& value();

    template<typename T>
    void set(T&& val)
    {
        Poco::Dynamic::Var tmp(val);
        fData.swap(tmp);
    }

    template<typename T>
    const T& extract()
    {
        if (value().type() != typeid(T))
        {
            throw RuntimeException::Builder(__FUNCTION__)
                    .append("Bad cast for property value")
                    .make<RuntimeException>();
        }
        return value().extract<T>();
    }

    template<typename T>
    bool isType()
    {
        return (value().type() == typeid(T));
    }

private:
    Poco::Dynamic::Var      fData;
};

class PropertyTree : public UniquePersistent<PropertyTree>
{
public:
    PropertyTree();
    ~PropertyTree();

    PropertyTreeNode *resolve(const std::string& path);
    inline PropertyTreeNode *operator()(const std::string& path)
    {
        return resolve(path);
    }

private:
    PropertyTreeDirNode     *fRoot;
};

} // namespace cocoa

#endif //COCOA_PROPERTY_TREE_H
