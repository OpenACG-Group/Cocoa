#ifndef COCOA_CORE_PROPERTIES_H
#define COCOA_CORE_PROPERTIES_H

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <any>
#include <functional>

#include "Core/Project.h"
#include "Core/Journal.h"

namespace cocoa {

class PropertyNode : public std::enable_shared_from_this<PropertyNode>
{
public:
    enum class Protection
    {
        kPublic,
        kPrivate,
        kDefault = kPrivate
    };

    enum class Kind {
        kObject,
        kArray,
        kData
    };

    using ForEachChildCb = std::function<void(const std::shared_ptr<PropertyNode>&, bool last)>;

    explicit PropertyNode(Kind kind);
    virtual ~PropertyNode() = default;

    template<typename T, typename std::enable_if<std::is_base_of_v<PropertyNode, T>>::type* = nullptr>
    co_nodiscard inline std::shared_ptr<T> as() {
        return std::dynamic_pointer_cast<T>(shared_from_this());
    }

    inline std::shared_ptr<PropertyNode> parent()
    { return fParent.lock(); }

    inline Kind kind()
    { return fKind; }

    std::shared_ptr<PropertyNode> next(uint32_t i0);
    std::shared_ptr<PropertyNode> next(const std::string& member);

    void setParent(const std::shared_ptr<PropertyNode>& node);

    void setProtection(Protection prot) {
        fProtection = prot;
    }

    co_nodiscard inline Protection protection() const
    { return fProtection; }
    co_nodiscard std::string getName();

    virtual std::string toQLogString() = 0;
    virtual void forEachChild(ForEachChildCb cb) = 0;

private:
    std::weak_ptr<PropertyNode>  fParent;
    Kind                         fKind;
    Protection                   fProtection;
};

class PropertyObjectNode : public PropertyNode
{
public:
    PropertyObjectNode();
    ~PropertyObjectNode() override = default;

    std::shared_ptr<PropertyNode> getMember(const std::string& name);
    std::shared_ptr<PropertyNode> setMember(const std::string& name,
                                            const std::shared_ptr<PropertyNode>& member);
    void renameMember(const std::string& oldName, const std::string& newName);
    void unsetMember(const std::string& name);
    bool hasMember(const std::string& name);

    co_nodiscard inline auto begin()
    { return fMembers.begin(); }

    co_nodiscard inline auto end()
    { return fMembers.end(); }

    std::string toQLogString() override;
    void forEachChild(ForEachChildCb cb) override {
        int32_t idx = 0;
        for (const auto& p : fMembers)
            cb(p.second, idx++ == fMembers.size() - 1);
    }

private:
    std::map<std::string, std::shared_ptr<PropertyNode>> fMembers;
};

class PropertyDataNode : public PropertyNode
{
public:
    explicit PropertyDataNode(std::any&& value);
    PropertyDataNode();
    ~PropertyDataNode() override = default;

    template<typename T>
    T& extract() {
        return std::any_cast<T&>(fData);
    }

    const std::type_info& type();
    void reset(std::any&& value);

    co_nodiscard inline bool hasValue() const {
        return fData.has_value();
    }

    std::string toQLogString() override;
    void forEachChild(ForEachChildCb cb) override {}

private:
    std::any    fData;
};

class PropertyArrayNode : public PropertyNode
{
public:
    PropertyArrayNode();
    ~PropertyArrayNode() override = default;

    template<typename T>
    std::shared_ptr<PropertyArrayNode> append(const std::vector<T>& vec) {
        for (const T& element : vec)
            append(std::make_shared<PropertyDataNode>(element));
        return shared_from_this()->template as<PropertyArrayNode>();
    }

    void append(std::shared_ptr<PropertyNode> node);
    void erase(uint32_t i0);
    std::shared_ptr<PropertyNode> at(uint32_t i0);

    co_nodiscard inline auto begin()
    { return fSubscripts.begin(); }

    co_nodiscard inline auto end()
    { return fSubscripts.end(); }

    co_nodiscard inline size_t size() const
    { return fSubscripts.size(); }

    std::string toQLogString() override;
    void forEachChild(ForEachChildCb cb) override {
        int32_t idx = 0;
        for (const auto& p : fSubscripts)
            cb(p, idx++ == fSubscripts.size() - 1);
    }

private:
    std::vector<std::shared_ptr<PropertyNode>> fSubscripts;
};

namespace prop
{

template<typename T>
inline std::shared_ptr<T> Cast(const std::shared_ptr<PropertyNode>& ptr) {
    return std::dynamic_pointer_cast<T>(ptr);
}

template<typename T, typename...ArgsT>
inline std::shared_ptr<T> New(ArgsT&&...args) {
    return std::make_shared<T>(std::forward<ArgsT>(args)...);
}

std::shared_ptr<PropertyObjectNode> Get();
void SerializeToJournal(const std::shared_ptr<PropertyObjectNode>& root);

} // namespace properties
} // namespace cocoa
#endif // COCOA_CORE_PROPERTIES_H
