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

#include <fstream>
#include <map>

#include "fmt/format.h"
#include "Core/ApplicationInfo.h"
#include "Core/Utils.h"
#include "Core/Filesystem.h"
#include "Gallium/Gallium.h"
#include "Gallium/ModuleImportURL.h"
#include "Gallium/Internals.h"
#include "Gallium/ffi/Module.h"
GALLIUM_NS_BEGIN

namespace {

std::string normalize(std::string url)
{
    if (url[0] != '/')
    {
        std::string cwd = ApplicationInfo::Ref().working_dir;
        cwd.push_back('/');
        cwd.append(url);
        url = cwd;
    }
    return utils::GetAbsoluteDirectory(url);
}

std::string resolve_rel_filepath(const std::string& base, const std::string& specifier)
{
    std::string result;
    if (specifier[0] == '/')
        result = specifier;
    else
    {
        auto where = base.find_last_of('/');
        if (where != std::string::npos)
            result = base.substr(0, where + 1);
        result.append(specifier);
    }
    return normalize(result);
}

// NOLINTNEXTLINE
std::map<std::string, ModuleImportURL::Protocol> protocols_ = {
        { "internal://",    ModuleImportURL::Protocol::kInternal    },
        { "native://",      ModuleImportURL::Protocol::kNative      },
        { "file://",        ModuleImportURL::Protocol::kFile        }
};

const char *possible_file_ext_[] = {"", ".js", ".mjs"};

const char *resolve_internal_script(const std::string& name, ModuleImportURL::ResolvedAs as)
{
    InternalScript::ScopeAttr scope;
    switch (as)
    {
    case ModuleImportURL::ResolvedAs::kUserExecute:
        scope = InternalScript::kUserExecute;
        break;
    case ModuleImportURL::ResolvedAs::kUserImport:
        scope = InternalScript::kUserImport;
        break;
    case ModuleImportURL::ResolvedAs::kSysExecute:
        scope = InternalScript::kSysExecute;
        break;
    case ModuleImportURL::ResolvedAs::kSysImport:
        scope = InternalScript::kSysImport;
        break;
    }
    auto [ptr, error] = InternalScript::Get(name, scope);
    switch (error)
    {
    case InternalScript::Error::kOutOfScope:
        throw RuntimeException(__func__,
                               fmt::format("Reference to internal script {} is out of scope",
                                           name));
    case InternalScript::Error::kNotFound:
        throw RuntimeException(__func__,
                               fmt::format("Internal script {} not found", name));
    default:
        return ptr->content;
    }
    MARK_UNREACHABLE();
}

} // namespace anonymous

ModuleImportURL::SharedPtr
ModuleImportURL::Resolve(v8::Isolate *isolate,
                         ModuleImportURL *referer,
                         const std::string &import,
                         ResolvedAs resolvedAs)
{
    /* Native modules are not allowed to import other modules */
    if (referer && referer->GetProtocol() == Protocol::kNative)
        return nullptr;

    Protocol proto = Protocol::kInvalid;
    std::string path;
    for (const auto& pproto : protocols_)
    {
        if (import.find(pproto.first, 0) == 0)
        {
            proto = pproto.second;
            path = import.substr(pproto.first.length());
            break;
        }
    }

    ffi::NativeModule *native_module = nullptr;
    const char *persistent_cache_text = nullptr;

    auto *registry = ffi::ModuleRegistry::FromIsolate(isolate);

    /* `import` URL doesn't start with an appropriate prefix */
    if (proto == Protocol::kInvalid)
    {
        /* 'internal://' must be specified explicitly */
        proto = Protocol::kFile;
        path = import;
        if ((native_module = registry->SearchModule(path)))
            proto = Protocol::kNative;
    }

    if (proto == Protocol::kNative && !native_module)
    {
        if (!(native_module = registry->SearchModule(path)))
            return nullptr;
    }
    else if (proto == Protocol::kFile)
    {
        if (referer && referer->GetProtocol() == Protocol::kFile)
            path = resolve_rel_filepath(referer->GetPath(), path);
        else
            path = normalize(path);
        bool ok = false;
        for (const char *postfix : possible_file_ext_)
        {
            std::string maybePath = path + postfix;
            if (vfs::Access(maybePath, {vfs::AccessMode::kRegular,
                                        vfs::AccessMode::kReadable}) == vfs::AccessResult::kOk)
            {
                ok = true;
                path = maybePath;
                break;
            }
        }
        if (!ok)
            return nullptr;
    }
    else if (proto == Protocol::kInternal)
    {
        if (!(persistent_cache_text = resolve_internal_script(path, resolvedAs)))
            return nullptr;
    }

    return std::make_shared<ModuleImportURL>(
            proto, path, native_module, persistent_cache_text);
}

std::string ModuleImportURL::OnLoadResourceText() const
{
    CHECK(protocol_ == Protocol::kFile);

    std::ifstream fs(path_);
    /* We've determined that `fPath` must be valid in `Resolve()` */
    CHECK(fs.is_open());
    std::string content((std::istreambuf_iterator<char>(fs)),
                        std::istreambuf_iterator<char>());
    return content;
}

void ModuleImportURL::FreeInternalCaches()
{
    InternalScript::GlobalCollect();
}

GALLIUM_NS_END
