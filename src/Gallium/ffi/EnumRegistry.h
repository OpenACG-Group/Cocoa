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

#ifndef COCOA_GALLIUM_FFI_ENUMREGISTRY_H
#define COCOA_GALLIUM_FFI_ENUMREGISTRY_H

#include <unordered_map>
#include <string>
#include <vector>
#include <set>

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ReturnValue.h"
GALLIUM_FFI_NS_BEGIN

class EnumRegistry
{
public:
    struct EnumInfo
    {
        EnumInfo(const std::string_view& name_, bool bitfield_)
            : name(name_), bitfield(bitfield_), min_item(0), max_item(0) {}

        EnumInfo(EnumInfo&& rhs) noexcept
            : name(std::move(rhs.name))
            , bitfield(rhs.bitfield)
            , items(std::move(rhs.items))
            , min_item(rhs.min_item)
            , max_item(rhs.max_item)
            , value_set(std::move(rhs.value_set)) {}

        std::string name;
        bool bitfield;
        std::unordered_map<std::string, int64_t> items;

        // Filled by `EnumRegistry`
        int64_t min_item;
        int64_t max_item;
        std::set<int64_t> value_set;
    };

    static EnumRegistry *FromIsolate(Isolate *isolate);

    explicit EnumRegistry(Isolate *isolate);

    Local<Object> AddEnum(const ClassTypeInfo& type_info, EnumInfo info);
    Ret<int64_t> CastEnumValueSafe(const ClassTypeInfo& type_info, Local<Value> value);

private:
    struct Hasher
    {
        size_t operator()(const ClassTypeInfo& tf) const {
            return tf.Hash();
        }
    };
    using TypeInfoMap = std::unordered_map<ClassTypeInfo, EnumInfo, Hasher>;

    Isolate         *isolate_;
    TypeInfoMap      type_info_map_;
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_ENUMREGISTRY_H
