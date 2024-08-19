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

#include "Gallium/ffi/ReturnValue.h"
GALLIUM_FFI_NS_BEGIN

void ReturnValueBase::Error::Throw(v8::Isolate *isolate) const
{
    if (type == ErrorType::kFreePropagate)
        // Error has been thrown, just return.
        return;

    v8::Local<v8::String> str = Cast<std::string>::ToChecked(isolate, message);
    v8::Local<v8::Value> object;
    switch (type)
    {
#define ENTRY(name) case ErrorType::k##name: object = v8::Exception::name(str); break;
        ENTRY(Error)
        ENTRY(RangeError)
        ENTRY(TypeError)
        ENTRY(ReferenceError)
        ENTRY(SyntaxError)
        ENTRY(WasmCompileError)
        ENTRY(WasmLinkError)
        ENTRY(WasmRuntimeError)
#undef ENTRY
    default:
        MARK_UNREACHABLE();
    }
    isolate->ThrowException(object);
}

ReturnValueBase::Error Fail(ErrorType type, const std::string_view& message)
{
    return { type, std::string(message) };
}

ReturnValueBase::Error Fail(const v8::TryCatch& try_catch, const std::string_view& prefix)
{
    if (!try_catch.HasCaught())
        return { kErr, "Unknown error" };

    // TODO(sora): Collect more detailed information
    Isolate *isolate = v8::Isolate::GetCurrent();
    std::string message = Cast<std::string>::FromChecked(isolate, try_catch.Message()->Get());
    return {
        kErr,
        fmt::format("{}{}", prefix, message)
    };
}

ReturnValueBase::Error FreePropagate()
{
    return { ErrorType::kFreePropagate, {} };
}

GALLIUM_FFI_NS_END
