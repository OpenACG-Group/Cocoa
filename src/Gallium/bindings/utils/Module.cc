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

#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/Function.h"
#include "Gallium/bindings/utils/Module.h"
#include "Gallium/bindings/utils/TextCodec.h"
GALLIUM_BINDINGS_NS_BEGIN

UtilsModule::UtilsModule()
    : ffi::NativeModule("utils", "Miscellaneous classes and functions")
{
}

#define EXPORT_FUNC(func)                                   \
    exports[#func] = ffi::WrapFunctionAddress(              \
        isolate, v8::SideEffectType::kHasNoSideEffect,      \
        utils::func)->GetFunction(context).ToLocalChecked();

ffi::LocalExports UtilsModule::OnBuildExports(v8::Isolate *isolate)
{
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    ffi::LocalExports exports;

    exports["TextCodec"] = ffi::DefineEnum<utils::TextCodec>(isolate, "TextCodec", false)
        .Item("Latin1", utils::TextCodec::kLatin1)
        .Item("UTF8", utils::TextCodec::kUTF8)
        .Item("UCS2", utils::TextCodec::kUCS2)
        .Item("Hex", utils::TextCodec::kHex)
        .Finalize();

    EXPORT_FUNC(computeTextByteSize)
    EXPORT_FUNC(encodeText)
    EXPORT_FUNC(encodeTextInto)
    EXPORT_FUNC(decodeText)

    return exports;
}

GALLIUM_BINDINGS_NS_END
