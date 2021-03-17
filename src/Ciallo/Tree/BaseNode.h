#ifndef COCOA_BASENODE_H
#define COCOA_BASENODE_H

#include <list>

#include "Core/Exception.h"
#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

class BaseNode;
template<typename T>
concept InheritGrBaseNode = std::is_base_of<BaseNode, T>::value;

#define CIALLO_STATIC_NODE_KIND(kind)   const static NodeKind NODE_KIND = kind;
class BaseNode
{
public:
    enum class NodeKind
    {
        kPaintNode,
        kRenderNode,
        kCompositeNode
    };

    enum class NodeBackendKind
    {
        kRaster,
        kGpu,
        kUnknown,
        kUndefined
    };

    class ChildrenIterable
    {
    public:
        explicit ChildrenIterable(std::list<BaseNode*>& node);
        ChildrenIterable(const ChildrenIterable& that);
        std::list<BaseNode*>::iterator begin();
        std::list<BaseNode*>::iterator end();

    private:
        std::list<BaseNode*>::iterator    fBegin;
        std::list<BaseNode*>::iterator    fEnd;
    };

    virtual ~BaseNode();

    inline NodeKind kind() const
    { return fKind; }

    inline BaseNode *parent()
    { return fParent; }

    ChildrenIterable children();
    void setParent(BaseNode *parent);

    /* Just append a new child node to current node,
       will not modify child's parent field. */
    void appendChild(const BaseNode *child);

    /* Just remove a child node to current node,
       will not modify child's parent field. */
    void removeChild(const BaseNode *child);

    template<InheritGrBaseNode T>
    inline T *cast()
    {
        if (this->kind() != T::NODE_KIND)
            this->badNodeCast();
        return dynamic_cast<T*>(this);
    }

    virtual NodeBackendKind backendKind() const;

protected:
    explicit BaseNode(NodeKind kind, BaseNode *parent = nullptr);
    void parentDispose();

private:
    void badNodeCast();

private:
    NodeKind                 fKind;
    NodeBackendKind          fBackendKind;
    BaseNode              *fParent;
    std::list<BaseNode*>   fChildrenList;
};

CIALLO_END_NS
#endif //COCOA_BASENODE_H
