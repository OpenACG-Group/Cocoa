#ifndef COCOA_MODULEIMPORTURL_H
#define COCOA_MODULEIMPORTURL_H

#include <string>
#include <optional>
#include <tuple>
#include "Core/Errors.h"

#include "include/v8.h"
#include "Koi/KoiBase.h"

KOI_BINDINGS_NS_BEGIN

class BindingBase;

KOI_BINDINGS_NS_END

KOI_NS_BEGIN

class ModuleImportURL
{
public:
    using UniquePtr = std::unique_ptr<ModuleImportURL>;

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
        kImport,
        kUserExecute,
        kEngineExecute
    };

    ModuleImportURL(Protocol protocol, std::string path,
                    bindings::BindingBase *bind = nullptr,
                    const char *persistentCachedText = nullptr)
            : fProtocol(protocol)
            , fPath(std::move(path))
            , fBinding(bind)
            , fPersistentCachedText(persistentCachedText) {}
    ~ModuleImportURL() = default;

    static std::unique_ptr<ModuleImportURL> Resolve(const std::unique_ptr<ModuleImportURL>& referer,
                                                    const std::string& import,
                                                    ResolvedAs resolvedAs);

    koi_nodiscard inline Protocol getProtocol() const {
        CHECK(fProtocol != Protocol::kInvalid);
        return fProtocol;
    }

    koi_nodiscard inline const std::string& getPath() const {
        CHECK(fProtocol != Protocol::kInvalid);
        return fPath;
    }

    koi_nodiscard inline std::string toString() const {
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

    koi_nodiscard inline std::optional<std::string> loadResourceText() const {
        CHECK(fProtocol != Protocol::kInvalid);
        if (fPersistentCachedText)
            return fPersistentCachedText;
        return (fProtocol == Protocol::kSynthetic
                ? std::optional<std::string>()
                : std::make_optional<std::string>(onLoadResourceText()));
    }

    koi_nodiscard bindings::BindingBase *getSyntheticBinding() const {
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
    koi_nodiscard std::string onLoadResourceText() const;

    Protocol                fProtocol;
    std::string             fPath;
    bindings::BindingBase  *fBinding;
    const char             *fPersistentCachedText;
};


KOI_NS_END
#endif //COCOA_MODULEIMPORTURL_H
