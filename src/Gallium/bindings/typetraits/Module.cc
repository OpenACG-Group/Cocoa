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

#include "Gallium/bindings/typetraits/Module.h"
#include "Gallium/bindings/typetraits/Exports.h"
#include "Gallium/ffi/Function.h"
GALLIUM_BINDINGS_NS_BEGIN

TypetraitsModule::TypetraitsModule()
    : NativeModule("typetraits", "Extended runtime type information")
{
}

ffi::LocalExports TypetraitsModule::OnBuildExports(v8::Isolate *isolate)
{
    using namespace typetraits;
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

#define EXPORT_IS_XXX(type) \
    { "Is" #type, ffi::WrapFunctionAddress(isolate, \
      v8::SideEffectType::kHasNoSideEffect, Is##type)->GetFunction(ctx).ToLocalChecked() \
    },

#define EXPORT_FUNC(func) \
    { #func, ffi::WrapFunctionAddress(isolate, \
      v8::SideEffectType::kHasNoSideEffect, func)->GetFunction(ctx).ToLocalChecked() \
    },

#define EXPORT_ENUM(name, v) \
    { #name, v8::Int32::New(isolate, static_cast<int32_t>(v)) },

    //! TSDecl: @enum Constants
    //! TSDecl: @enumitem PROPERTY_FILTER_ALL_PROPERTIES
    //! TSDecl: @enumitem PROPERTY_FILTER_ONLY_WRITABLE
    //! TSDecl: @enumitem PROPERTY_FILTER_ONLY_ENUMERABLE
    //! TSDecl: @enumitem PROPERTY_FILTER_ONLY_CONFIGURABLE
    //! TSDecl: @enumitem PROPERTY_FILTER_SKIP_STRINGS
    //! TSDecl: @enumitem PROPERTY_FILTER_SKIP_SYMBOLS
    //! TSDecl: @enumitem PROMISE_STATE_FULFILLED
    //! TSDecl: @enumitem PROMISE_STATE_PENDING
    //! TSDecl: @enumitem PROMISE_STATE_REJECTED
    //! TSDecl: @end
    ffi::LocalExports constants{
        EXPORT_ENUM(PROPERTY_FILTER_ALL_PROPERTIES, v8::PropertyFilter::ALL_PROPERTIES)
        EXPORT_ENUM(PROPERTY_FILTER_ONLY_WRITABLE, v8::PropertyFilter::ONLY_WRITABLE)
        EXPORT_ENUM(PROPERTY_FILTER_ONLY_ENUMERABLE, v8::PropertyFilter::ONLY_ENUMERABLE)
        EXPORT_ENUM(PROPERTY_FILTER_ONLY_CONFIGURABLE, v8::PropertyFilter::ONLY_CONFIGURABLE)
        EXPORT_ENUM(PROPERTY_FILTER_SKIP_STRINGS, v8::PropertyFilter::SKIP_STRINGS)
        EXPORT_ENUM(PROPERTY_FILTER_SKIP_SYMBOLS, v8::PropertyFilter::SKIP_SYMBOLS)
        EXPORT_ENUM(PROMISE_STATE_FULFILLED, v8::Promise::PromiseState::kFulfilled)
        EXPORT_ENUM(PROMISE_STATE_PENDING, v8::Promise::PromiseState::kPending)
        EXPORT_ENUM(PROMISE_STATE_REJECTED, v8::Promise::PromiseState::kRejected)
    };

    return ffi::LocalExports {
        TYPES_METHOD_MAP(EXPORT_IS_XXX)
        EXPORT_IS_XXX(AnyArrayBuffer)
        EXPORT_IS_XXX(BoxedPrimitive)
        EXPORT_FUNC(GetOwnNonIndexProperties)
        EXPORT_FUNC(GetConstructorName)
        EXPORT_FUNC(GetPromiseDetails)
        EXPORT_FUNC(GetProxyDetails)
        EXPORT_FUNC(PreviewEntries)
        { "Constants", ffi::Cast<ffi::LocalExports>::ToChecked(isolate, constants) }
    };
}

GALLIUM_BINDINGS_NS_END
