#ifndef COCOA_PLISTASTNODE_H
#define COCOA_PLISTASTNODE_H

#include "Core/Project.h"

#include <cstdint>
#include <utility>
#include <vector>
#include "Core/Errors.h"
#include <memory>

namespace cocoa
{

class PListValue;

class PListASTNode
{
public:
    explicit PListASTNode(std::weak_ptr<PListASTNode> parent)
        : fParent(std::move(parent))
    {
    }
    virtual ~PListASTNode() = default;

    co_nodiscard inline std::shared_ptr<PListASTNode> parent()
    { return fParent.lock(); }

    co_nodiscard inline std::shared_ptr<PListASTNode> child(int idx)
    {
        CHECK(idx < fChildren.size());
        return fChildren[idx];
    }

    inline void setParent(std::weak_ptr<PListASTNode> parent)
    { fParent = std::move(parent); }

    inline void setChild(int idx, const std::shared_ptr<PListASTNode>& child)
    {
        CHECK(idx < fChildren.size() && child);
        fChildren[idx] = child;
    }

    inline void appendChild(const std::shared_ptr<PListASTNode>& child)
    {
        CHECK(child);
        fChildren.push_back(child);
    }

    inline void removeChild(const std::shared_ptr<PListASTNode>& child)
    {
        auto itr = std::find(fChildren.begin(), fChildren.end(), child);
        if (itr != fChildren.end())
            fChildren.erase(itr);
    }

    inline void removeChild(int idx)
    {
        CHECK(idx > fChildren.size());
        fChildren.erase(idx);
    }

    virtual std::shared_ptr<PListValue> perform() = 0;

private:
    std::weak_ptr<PListASTNode> fParent;
    std::vector<std::shared_ptr<PListASTNode>> fChildren;
};

} // namespace cocoa
#endif //COCOA_PLISTASTNODE_H
