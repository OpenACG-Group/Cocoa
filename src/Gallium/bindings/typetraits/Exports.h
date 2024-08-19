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

#ifndef COCOA_GALLIUM_BINDINGS_TYPES_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_TYPES_EXPORTS_H

#include "include/v8.h"
#include "Gallium/ffi/Module.h"
#include "Gallium/ffi/ReturnValue.h"

#define GALLIUM_BINDINGS_TYPETRAITS_NS_BEGIN namespace cocoa::gallium::bindings::typetraits {
#define GALLIUM_BINDINGS_TYPETRAITS_NS_END   }

GALLIUM_BINDINGS_TYPETRAITS_NS_BEGIN

#define TYPES_METHOD_MAP(V)              \
    V(External)                          \
    V(TypedArray)                        \
    V(Date)                              \
    V(ArgumentsObject)                   \
    V(BigIntObject)                      \
    V(BooleanObject)                     \
    V(NumberObject)                      \
    V(StringObject)                      \
    V(SymbolObject)                      \
    V(NativeError)                       \
    V(RegExp)                            \
    V(AsyncFunction)                     \
    V(GeneratorFunction)                 \
    V(GeneratorObject)                   \
    V(Promise)                           \
    V(Map)                               \
    V(Set)                               \
    V(MapIterator)                       \
    V(SetIterator)                       \
    V(WeakMap)                           \
    V(WeakSet)                           \
    V(ArrayBuffer)                       \
    V(DataView)                          \
    V(SharedArrayBuffer)                 \
    V(Proxy)                             \
    V(ModuleNamespaceObject)

#define FUNC_DECL(type)                      \
    ffi::Ret<bool> Is##type(v8::Local<v8::Value> v);

//! TSDecl: @function IsExternal(value: any): boolean
//! TSDecl: @function IsTypedArray(value: any): boolean
//! TSDecl: @function IsDate(value: any): boolean
//! TSDecl: @function IsArgumentsObject(value: any): boolean
//! TSDecl: @function IsBigIntObject(value: any): boolean
//! TSDecl: @function IsBooleanObject(value: any): boolean
//! TSDecl: @function IsNumberObject(value: any): boolean
//! TSDecl: @function IsStringObject(value: any): boolean
//! TSDecl: @function IsSymbolObject(value: any): boolean
//! TSDecl: @function IsNativeError(value: any): boolean
//! TSDecl: @function IsRegExp(value: any): boolean
//! TSDecl: @function IsAsyncFunction(value: any): boolean
//! TSDecl: @function IsGeneratorFunction(value: any): boolean
//! TSDecl: @function IsGeneratorObject(value: any): boolean
//! TSDecl: @function IsPromise(value: any): boolean
//! TSDecl: @function IsMap(value: any): boolean
//! TSDecl: @function IsSet(value: any): boolean
//! TSDecl: @function IsMapIterator(value: any): boolean
//! TSDecl: @function IsSetIterator(value: any): boolean
//! TSDecl: @function IsWeakMap(value: any): boolean
//! TSDecl: @function IsWeakSet(value: any): boolean
//! TSDecl: @function IsArrayBuffer(value: any): boolean
//! TSDecl: @function IsDataView(value: any): boolean
//! TSDecl: @function IsSharedArrayBuffer(value: any): boolean
//! TSDecl: @function IsProxy(value: any): boolean
//! TSDecl: @function IsModuleNamespaceObject(value: any): boolean

TYPES_METHOD_MAP(FUNC_DECL)

#undef FUNC_DECL

//! TSDecl: @function IsAnyArrayBuffer(value: any): boolean
ffi::Ret<bool> IsAnyArrayBuffer(v8::Local<v8::Value> v);

//! TSDecl: @function IsAnyArrayBuffer(value: any): boolean
ffi::Ret<bool> IsBoxedPrimitive(v8::Local<v8::Value> v);

//! TSDecl: @function GetOwnNonIndexProperties(obj: object, filter: u32): @array(string)
ffi::RetLocal<v8::Value> GetOwnNonIndexProperties(v8::Local<v8::Object> obj, int32_t filter);

//! TSDecl: @function GetConstructorName(obj: object): string
ffi::RetLocal<v8::Value> GetConstructorName(v8::Local<v8::Object> obj);

//! TSDecl: @interface PromiseDetails
//! TSDecl: @property state: u32
//! TSDecl: @property @optional result: any
//! TSDecl: @end

//! TSDecl: @function GetPromiseDetails(promise: @generic(Promise, any)): PromiseDetails
ffi::RetLocal<v8::Value> GetPromiseDetails(v8::Local<v8::Promise> promise);

//! TSDecl: @interface ProxyDetails
//! TSDecl: @property target: any
//! TSDecl: @property handler: any
//! TSDecl: @end

//! TSDecl: @function GetProxyDetails(proxy: object): ProxyDetails
ffi::RetLocal<v8::Value> GetProxyDetails(v8::Local<v8::Proxy> proxy);

//! TSDecl: @interface EntriesInfo
//! TSDecl: @property entries: @array(any)
//! TSDecl: @property isKeyValue: boolean
//! TSDecl: @end

//! TSDecl: @function PreviewEntries(obj: object): EntriesInfo
ffi::RetLocal<v8::Value> PreviewEntries(v8::Local<v8::Object> obj);

GALLIUM_BINDINGS_TYPETRAITS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_TYPES_EXPORTS_H
