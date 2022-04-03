#ifndef COCOA_COBALT_LAPSCOMPILER_PARSER_ASTBASE_H
#define COCOA_COBALT_LAPSCOMPILER_PARSER_ASTBASE_H

#include <list>
#include <map>

#include "Core/Errors.h"
#include "Cobalt/LapsCompiler/LapsCompiler.h"
LAPS_COMPILER_BEGIN_NS

class location;

#define AST_ASSERT_TYPE(node, type) \
    CHECK((node) && (node)->GetNodeType() == ASTNodeBase::Type::k##type)

#define AST_DECL_CHILDID(name, value)                               \
    constexpr static ChildNodeID kChild##name = value;              \
    g_nodiscard g_inline co_sp<AST##name##Node> GetChild##name() {  \
        return GetChildNode(kChild##name)->Cast<AST##name##Node>(); \
    }

class ASTNodeBase : public std::enable_shared_from_this<ASTNodeBase>
{
public:
    using ChildNodeID = uint32_t;

    enum class Type
    {
        kTranslationUnit,
        kStatementList,
        kLiteralValue,
        kTypeNameExpr,
        kLetDeclarationStmt,
        kDeclAttrSpecList,
        kDeclAttrSpec,
        kDeclAttrSpecLiteralList
    };

    struct Location
    {
        Location() = default;
        explicit Location(const location& loc);
        ~Location() = default;

        g_nodiscard std::string ToString() const;

        int32_t         start_line = -1;
        int32_t         start_column = -1;
        int32_t         end_line = -1;
        int32_t         end_column = -1;
    };

    using Ptr = co_sp<ASTNodeBase>;

    template<typename T, typename...ArgsT>
    g_nodiscard g_inline static co_sp<T> New(ArgsT&&... args) {
        auto r = std::make_shared<T>(std::forward<ArgsT>(args)...);
        r->AcceptDeferredSetParentNodes();
        return r;
    }

    ASTNodeBase(Type type, const location& loc);
    explicit ASTNodeBase(Type type);
    virtual ~ASTNodeBase() = default;

    template<typename T>
    g_nodiscard g_inline co_sp<T> Cast() {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    g_nodiscard g_inline const Location& GetLocationInfo() const {
        return location_info_;
    }

    g_nodiscard g_inline Type GetNodeType() const {
        return node_type_;
    }

    g_nodiscard g_inline co_sp<ASTNodeBase> GetParentNode() const {
        return parent_node_.lock();
    }

    g_nodiscard g_inline co_sp<ASTNodeBase> Self() {
        try {
            return shared_from_this();
        } catch (const std::bad_weak_ptr& e) {
            return nullptr;
        }
    }

    g_nodiscard co_sp<ASTNodeBase> GetChildNode(ChildNodeID id);
    void SetChildNode(ChildNodeID id, const co_sp<ASTNodeBase>& child);

    g_inline void ForeachChildNode(const std::function<void(const co_sp<ASTNodeBase>&)>& func) {
        for (const auto& pair : child_nodes_)
            func(pair.second);
    }

private:
    void AcceptDeferredSetParentNodes();

    Type                                            node_type_;
    Location                                        location_info_;
    co_weak<ASTNodeBase>                            parent_node_;
    std::map<ChildNodeID, co_sp<ASTNodeBase>>       child_nodes_;
    std::vector<co_sp<ASTNodeBase>>                 deferred_set_parent_nodes_;
};

LAPS_COMPILER_END_NS
#endif //COCOA_COBALT_LAPSCOMPILER_PARSER_ASTBASE_H
