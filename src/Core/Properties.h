#ifndef COCOA_PROPERTIES_H
#define COCOA_PROPERTIES_H

#include <vector>
#include <string>
#include <memory>
#include <map>
#include <any>

namespace cocoa {

class PropertyNode : public std::enable_shared_from_this<PropertyNode>
{
public:
    enum class Kind {
        kObject,
        kArray,
        kData
    };

    explicit PropertyNode(Kind kind);
    virtual ~PropertyNode() = default;

    inline std::shared_ptr<PropertyNode> parent()
    { return fParent.lock(); }

    inline Kind kind()
    { return fKind; }

    std::shared_ptr<PropertyNode> next(uint32_t i0);
    std::shared_ptr<PropertyNode> next(const std::string& member);

    void setParent(const std::shared_ptr<PropertyNode>& node);

private:
    std::weak_ptr<PropertyNode>              fParent;
    Kind                                     fKind;
};

class PropertyObjectNode : public PropertyNode
{
public:
    PropertyObjectNode();
    ~PropertyObjectNode() override = default;

    std::shared_ptr<PropertyNode> getMember(const std::string& name);
    void setMember(const std::string& name, std::shared_ptr<PropertyNode> member);
    void unsetMember(const std::string& name);

private:
    std::map<std::string, std::shared_ptr<PropertyNode>> fMembers;
};

class PropertyArrayNode : public PropertyNode
{
public:
    PropertyArrayNode();
    ~PropertyArrayNode() override = default;

    void append(std::shared_ptr<PropertyNode> node);
    void erase(uint32_t i0);
    std::shared_ptr<PropertyNode> at(uint32_t i0);

private:
    std::vector<std::shared_ptr<PropertyNode>> fSubscripts;
};

class PropertyDataNode : public PropertyNode
{
public:
    explicit PropertyDataNode(std::any&& value);
    ~PropertyDataNode() override = default;

    template<typename T>
    T& extract() {
        return std::any_cast<T&>(fData);
    }

    const std::type_info& type();
    void reset(std::any&& value);

private:
    std::any    fData;
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

} // namespace properties
} // namespace cocoa
#endif // COCOA_PROPERTIES_H
