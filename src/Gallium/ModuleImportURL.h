#ifndef COCOA_GALLIUM_MODULEIMPORTURL_H
#define COCOA_GALLIUM_MODULEIMPORTURL_H

#include <string>
#include <optional>
#include <tuple>
#include "Core/Errors.h"

#include "include/v8.h"
#include "Gallium/Gallium.h"

GALLIUM_BINDINGS_NS_BEGIN

class BindingBase;

GALLIUM_BINDINGS_NS_END

GALLIUM_NS_BEGIN

class ModuleImportURL
{
public:
    using UniquePtr = std::unique_ptr<ModuleImportURL>;
    using SharedPtr = std::shared_ptr<ModuleImportURL>;

    enum class Protocol
    {
        /* Synthetic modules consists of native symbols */
        kSynthetic,
        /* Internal modules contains internal source code */
        kInternal,
        /* Absolute path is required */
        kFile,

        /* Object was moved */
        kInvalid
    };

    enum class ResolvedAs
    {
        kUserExecute,
        kUserImport,
        kSysExecute,
        kSysImport
    };

    ModuleImportURL(Protocol protocol, std::string path,
                    bindings::BindingBase *bind = nullptr,
                    const char *persistentCachedText = nullptr)
            : fProtocol(protocol)
            , fPath(std::move(path))
            , fBinding(bind)
            , fPersistentCachedText(persistentCachedText) {}
    ~ModuleImportURL() = default;

    static SharedPtr Resolve(const SharedPtr& referer, const std::string& import, ResolvedAs resolvedAs);

    static void FreeInternalCaches();

    g_nodiscard inline Protocol getProtocol() const {
        CHECK(fProtocol != Protocol::kInvalid);
        return fProtocol;
    }

    g_nodiscard inline const std::string& getPath() const {
        CHECK(fProtocol != Protocol::kInvalid);
        return fPath;
    }

    g_nodiscard inline std::string toString() const {
        CHECK(fProtocol != Protocol::kInvalid);
        std::string url;
        switch (fProtocol)
        {
        case Protocol::kFile:
            url = "file://";
            break;
        case Protocol::kSynthetic:
            url = "synthetic://";
            break;
        case Protocol::kInternal:
            url = "internal://";
            break;
        default:
            return {};
        }
        return (url + fPath);
    }

    g_nodiscard inline std::optional<std::string> loadResourceText() const {
        CHECK(fProtocol != Protocol::kInvalid);
        if (fPersistentCachedText)
            return fPersistentCachedText;
        return (fProtocol == Protocol::kSynthetic
                ? std::optional<std::string>()
                : std::make_optional<std::string>(onLoadResourceText()));
    }

    g_nodiscard bindings::BindingBase *getSyntheticBinding() const {
        return fBinding;
    }

    bool operator==(const ModuleImportURL& other) const {
        CHECK(fProtocol != Protocol::kInvalid);
        return (fProtocol == other.fProtocol && fPath == other.fPath);
    }

    bool operator<(const ModuleImportURL& other) const {
        CHECK(fProtocol != Protocol::kInvalid);
        return (toString() < other.toString());
    }

    bool operator>(const ModuleImportURL& other) const {
        CHECK(fProtocol != Protocol::kInvalid);
        return (toString() > other.toString());
    }

private:
    g_nodiscard std::string onLoadResourceText() const;

    Protocol                fProtocol;
    std::string             fPath;
    bindings::BindingBase  *fBinding;
    const char             *fPersistentCachedText;
};


GALLIUM_NS_END
#endif //COCOA_GALLIUM_MODULEIMPORTURL_H
