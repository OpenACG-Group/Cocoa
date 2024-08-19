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

#ifndef COCOA_GALLIUM_MODULEIMPORTURL_H
#define COCOA_GALLIUM_MODULEIMPORTURL_H

#include <string>
#include <optional>
#include <tuple>
#include "Core/Errors.h"

#include "include/v8.h"
#include "Gallium/Gallium.h"

GALLIUM_FFI_NS_BEGIN

class NativeModule;

GALLIUM_FFI_NS_END

GALLIUM_NS_BEGIN

class ModuleImportURL
{
public:
    using UniquePtr = std::unique_ptr<ModuleImportURL>;
    using SharedPtr = std::shared_ptr<ModuleImportURL>;

    enum class Protocol
    {
        kNative,
        /* Internal modules contains internal source code */
        kInternal,
        /* Absolute path is required */
        kFile,
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
                    ffi::NativeModule *native_module = nullptr,
                    const char *persistent_cache_text = nullptr)
            : protocol_(protocol)
            , path_(std::move(path))
            , native_module_(native_module)
            , persistent_cache_text_(persistent_cache_text) {}
    ~ModuleImportURL() = default;

    static SharedPtr Resolve(v8::Isolate *isolate,
                             ModuleImportURL *referer,
                             const std::string& import,
                             ResolvedAs resolvedAs);

    static void FreeInternalCaches();

    g_nodiscard Protocol GetProtocol() const {
        CHECK(protocol_ != Protocol::kInvalid);
        return protocol_;
    }

    g_nodiscard const std::string& GetPath() const {
        CHECK(protocol_ != Protocol::kInvalid);
        return path_;
    }

    g_nodiscard ffi::NativeModule *GetNativeModule() const {
        CHECK(protocol_ == Protocol::kNative);
        return native_module_;
    }

    g_nodiscard std::string ToString() const {
        CHECK(protocol_ != Protocol::kInvalid);
        std::string url;
        switch (protocol_)
        {
        case Protocol::kFile:
            url = "file://";
            break;
        case Protocol::kNative:
            url = "native://";
            break;
        case Protocol::kInternal:
            url = "internal://";
            break;
        default:
            return {};
        }
        return (url + path_);
    }

    g_nodiscard std::optional<std::string> LoadResourceText() const {
        CHECK(protocol_ != Protocol::kInvalid);
        if (persistent_cache_text_)
            return persistent_cache_text_;
        return (protocol_ == Protocol::kNative
                ? std::optional<std::string>()
                : std::make_optional<std::string>(OnLoadResourceText()));
    }

    bool operator==(const ModuleImportURL& other) const {
        CHECK(protocol_ != Protocol::kInvalid);
        return (protocol_ == other.protocol_ && path_ == other.path_);
    }

    bool operator<(const ModuleImportURL& other) const {
        CHECK(protocol_ != Protocol::kInvalid);
        return (ToString() < other.ToString());
    }

    bool operator>(const ModuleImportURL& other) const {
        CHECK(protocol_ != Protocol::kInvalid);
        return (ToString() > other.ToString());
    }

private:
    g_nodiscard std::string OnLoadResourceText() const;

    Protocol                protocol_;
    std::string             path_;
    ffi::NativeModule      *native_module_;
    const char             *persistent_cache_text_;
};


GALLIUM_NS_END
#endif //COCOA_GALLIUM_MODULEIMPORTURL_H
