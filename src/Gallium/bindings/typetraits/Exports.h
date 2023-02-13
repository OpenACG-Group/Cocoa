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

#define GALLIUM_BINDINGS_TYPETRAITS_NS_BEGIN namespace cocoa::gallium::bindings::typetraits {
#define GALLIUM_BINDINGS_TYPETRAITS_NS_END   }

GALLIUM_BINDINGS_TYPETRAITS_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

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
    bool Is##type(v8::Local<v8::Value> v);

//! TSDecl: function IsSomeType(value: any): boolean

TYPES_METHOD_MAP(FUNC_DECL)

#undef FUNC_DECL

bool IsAnyArrayBuffer(v8::Local<v8::Value> v);
bool IsBoxedPrimitive(v8::Local<v8::Value> v);

//! TSDecl: function GetOwnNonIndexProperties(obj: object, filter: Bitfield<PropertyFilter>): string[]
v8::Local<v8::Value> GetOwnNonIndexProperties(v8::Local<v8::Value> obj, int32_t filter);

//! TSDecl: function GetConstructorName(obj: object): string
v8::Local<v8::Value> GetConstructorName(v8::Local<v8::Value> obj);

//! TSDecl: function GetPromiseDetails(promise: Promise): {state: Enum<PromiseState>, result?: any}
v8::Local<v8::Value> GetPromiseDetails(v8::Local<v8::Value> promise);

//! TSDecl: function GetProxyDetails(proxy: Proxy): {target: any, handler: any}
v8::Local<v8::Value> GetProxyDetails(v8::Local<v8::Value> proxy);

//! TSDecl: function PreviewEntries(obj: object): {entries: any[], isKeyValue: boolean}
v8::Local<v8::Value> PreviewEntries(v8::Local<v8::Value> obj);

GALLIUM_BINDINGS_TYPETRAITS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_TYPES_EXPORTS_H
