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

#include <map>

#include <unistd.h>
#include <fcntl.h>

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Function.h"
#include "Gallium/bindings/fs/Module.h"
#include "Gallium/bindings/fs/FileOps.h"
GALLIUM_BINDINGS_NS_BEGIN

#define MODES_MAP(V) \
    V(S_IRWXU)       \
    V(S_IRUSR)       \
    V(S_IWUSR)       \
    V(S_IXUSR)       \
    V(S_IRWXG)       \
    V(S_IRGRP)       \
    V(S_IWGRP)       \
    V(S_IXGRP)       \
    V(S_IRWXO)       \
    V(S_IROTH)       \
    V(S_IWOTH)       \
    V(S_IXOTH)       \
    V(S_ISUID)       \
    V(S_ISGID)       \
    V(S_ISVTX)

FsModule::FsModule()
    : ffi::NativeModule("fs", "Filesystem operations")
{
}

ffi::LocalExports FsModule::OnBuildExports(v8::Isolate *isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    ffi::LocalExports exports;

    //! TSDecl: @enum Mode
    //! TSDecl: @enumitem S_IRWXU
    //! TSDecl: @enumitem S_IRUSR
    //! TSDecl: @enumitem S_IWUSR
    //! TSDecl: @enumitem S_IXUSR
    //! TSDecl: @enumitem S_IRWXG
    //! TSDecl: @enumitem S_IRGRP
    //! TSDecl: @enumitem S_IWGRP
    //! TSDecl: @enumitem S_IXGRP
    //! TSDecl: @enumitem S_IRWXO
    //! TSDecl: @enumitem S_IROTH
    //! TSDecl: @enumitem S_IWOTH
    //! TSDecl: @enumitem S_IXOTH
    //! TSDecl: @enumitem S_ISUID
    //! TSDecl: @enumitem S_ISGID
    //! TSDecl: @enumitem S_ISVTX
    //! TSDecl: @end
    exports["Mode"] = ffi::Cast<std::map<std::string, mode_t>>::ToChecked(isolate, {
#define ENTRY(x) { #x, x },
        MODES_MAP(ENTRY)
#undef ENTRY
    });

#define EXPORT_FUNC(func)                                   \
    exports[#func] = ffi::WrapFunctionAddress(              \
        isolate, v8::SideEffectType::kHasNoSideEffect,      \
        fs::func)->GetFunction(context).ToLocalChecked();

    EXPORT_FUNC(ReadFile)
    EXPORT_FUNC(WriteFile)
    EXPORT_FUNC(realpath)

    return exports;
}

GALLIUM_BINDINGS_NS_END
