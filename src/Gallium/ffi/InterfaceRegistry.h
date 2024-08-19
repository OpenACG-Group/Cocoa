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

#ifndef COCOA_GALLIUM_FFI_INTERFACEREGISTRY_H
#define COCOA_GALLIUM_FFI_INTERFACEREGISTRY_H

#include <vector>
#include <string>
#include <unordered_map>

#include "Gallium/Gallium.h"
#include "Gallium/ffi/TypeTraits.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ReturnValue.h"
GALLIUM_FFI_NS_BEGIN

class InterfaceRegistry
{
public:
    using FieldSetterF = Ret<void>(*)(Isolate*, void*, Local<Value>, v8::EscapableHandleScope&);
    using FieldGetterF = RetLocal<Value>(*)(Isolate*, const void*);

    struct PerFieldInfo
    {
        std::string iface_name;
        std::string name;
        bool optional;
        bool search_prototype;
        bool readonly;
        uint64_t offset;
        FieldSetterF setter;
        FieldGetterF getter;
    };
    using FieldInfoVec = std::vector<PerFieldInfo>;

    static InterfaceRegistry *FromIsolate(Isolate *isolate);

    explicit InterfaceRegistry(Isolate *isolate);
    ~InterfaceRegistry() = default;

    void AddInterface(const ClassTypeInfo& type_info, FieldInfoVec fields);

    Ret<void> FillStruct(const ClassTypeInfo& type_info, void *addr, Local<Object> object);
    RetLocal<Object> FillObject(const ClassTypeInfo& type_info, const void *addr);

private:
    struct Hasher
    {
        size_t operator()(const ClassTypeInfo& tf) const {
            return tf.Hash();
        }
    };
    using TypeInfoMap = std::unordered_map<ClassTypeInfo, FieldInfoVec, Hasher>;

    Isolate         *isolate_;
    TypeInfoMap      type_info_map_;
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_INTERFACEREGISTRY_H
