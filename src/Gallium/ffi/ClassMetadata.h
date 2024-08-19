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

#ifndef COCOA_GALLIUM_FFI_CLASSMETADATA_H
#define COCOA_GALLIUM_FFI_CLASSMETADATA_H

#include <map>
#include <unordered_map>

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
GALLIUM_FFI_NS_BEGIN

class ClassRegistry;
class JSObject;

struct ClassMetadata
{
    constexpr static int kFields_count = 2;
    constexpr static int kField_base_ptr = 0;
    constexpr static int kField_ptr = 1;

    struct Hasher
    {
        size_t operator()(const ClassTypeInfo& tf) const {
            return tf.Hash();
        }
    };

    struct InstanceInfo
    {
        JSObject    *base_ptr;

        // A type-erased pointer to the last instance in the inheritance chain:
        // JSObject (base)   <---- base_ptr
        //    ^
        //    |
        //   ...
        //    |
        //  Child1 (parent)  <---- parent pointer
        //    ^                         ^
        //    |                         | cast through `parent_cast_pfn()`
        //  Child2 (final)   <---- instance_ptr
        void        *instance_ptr;
    };

    using Map = std::unordered_map<ClassTypeInfo, ClassMetadata, Hasher>;
    using InstanceMap = std::multimap<ClassMetadata*, InstanceInfo>;

    using Deleter = void(*)(void *instance_ptr);
    using ToParentCastF = void*(*)(void *derived);

    enum Attribute
    {
        kNone_Attr = 0,

        /**
         * A transferable object is an object whose instance internal data can be
         * "moved out" and then be used to construct another new instance, and
         * the old instance becomes invalid.
         * It is allowed to transfer a transferable object between different V8
         * Contexts and Isolates. When a transferable object is transferred from
         * an Isolate (A) to another Isolate (B), the instance in Isolate A will
         * become invalid, and a new instance in Isolate B will be constructed.
         */
        kTransferable_Attr = 0x01,

        /**
         * A cloneable object is an object whose instance internal data can be
         * copied to construct another new instance.
         * It is allowed to transfer a cloneable object between different V8
         * Contexts and Isolates.
         */
        kCloneable_Attr = 0x02,

        kMessagePort_Attr = 0x04
    };

    ClassTypeInfo               class_type_info;
    ClassRegistry              *class_registry;
    uint32_t                    attrs;
    ClassMetadata              *parent_metadata;
    ToParentCastF               parent_cast_pfn;
    Global<FunctionTemplate>    ctor_func_template;
    Global<FunctionTemplate>    class_func_template;
    Deleter                     deleter;
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_CLASSMETADATA_H
