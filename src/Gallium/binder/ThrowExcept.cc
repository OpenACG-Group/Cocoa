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

#include "Core/Errors.h"
#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/binder/Convert.h"

GALLIUM_BINDER_NS_BEGIN

v8::Local<v8::Value> throw_(v8::Isolate* isolate, std::string_view str)
{
    return isolate->ThrowException(to_v8(isolate, str));
}

v8::Local<v8::Value> throw_(v8::Isolate* isolate, std::string_view str, ExceptionBuilderF builder)
{
    return isolate->ThrowException(builder(to_v8(isolate, str), /* options= */ {}));
}

void JSException::Throw(Category category, const std::string& what, v8::Isolate *isolate)
{
    if (isolate == nullptr)
        isolate = v8::Isolate::GetCurrent();
    throw JSException(isolate, what, category);
}

v8::Local<v8::Value> JSException::TakeOver(const JSException& except)
{
    return except.isolate_->ThrowException(except.asException());
}

v8::Local<v8::Value> JSException::asException() const
{
    v8::Local<v8::String> message = to_v8(isolate_, this->what());
    switch (category_)
    {
    case Category::kError:          return v8::Exception::Error(message);
    case Category::kRangeError:     return v8::Exception::RangeError(message);
    case Category::kTypeError:      return v8::Exception::TypeError(message);
    case Category::kReferenceError: return v8::Exception::ReferenceError(message);
    case Category::kSyntaxError:    return v8::Exception::SyntaxError(message);
    case Category::kWasmCompileError: return v8::Exception::WasmCompileError(message);
    case Category::kWasmLinkError:  return v8::Exception::WasmLinkError(message);
    case Category::kWasmRuntimeError: return v8::Exception::WasmRuntimeError(message);
    }
    MARK_UNREACHABLE("Unexpected enumeration value");
}

GALLIUM_BINDER_NS_END
