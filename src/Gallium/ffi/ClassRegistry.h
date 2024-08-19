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

#ifndef COCOA_GALLIUM_FFI_CLASSREGISTRY_H
#define COCOA_GALLIUM_FFI_CLASSREGISTRY_H

#include "include/v8.h"

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/ClassMetadata.h"
GALLIUM_FFI_NS_BEGIN

class JSObject;

class ClassRegistry
{
public:
    explicit ClassRegistry(Isolate *isolate);
    ~ClassRegistry();

    static ClassRegistry *Get(Isolate *isolate);

    void Dispose();

    /**
     * Register a C++ class with necessary reflection information.
     * The type of the class is described by `type_info` parameter, and the same type
     * info can be used for `WrapObject()` and `UnwrapObject` method to wrap or unwrap
     * an instance of that class.
     * If `parent_type_info` and `parent_cast_pfn` are provided, the class is considered
     * to be a derived class of a parent class specified by `parent_type_info`.
     * `parent_cast_pfn` should be a function that casts a derived class's pointer to its
     * parent class's pointer. To obey the JS principle, a class only can have a single
     * parent class. Note that the parent class must be a class that has been registered.
     */
    void AddClass(const ClassTypeInfo& type_info,
                  int attrs,
                  const std::optional<ClassTypeInfo>& parent_type_info,
                  ClassMetadata::ToParentCastF parent_cast_pfn,
                  Local<FunctionTemplate> ctor_func_template,
                  ClassMetadata::Deleter deleter);

    Local<Object> WrapObject(const ClassTypeInfo& type_info, JSObject *base_ptr, void *ptr);
    void *UnwrapObject(const ClassTypeInfo& type_info, Local<Object> object);
    JSObject *UnwrapObjectBase(Local<Object> object);

private:
    void DeleteInstance(JSObject *base_ptr, void *instance_ptr);

    Isolate                    *isolate_;
    ClassMetadata::Map          class_info_map_;
    ClassMetadata::InstanceMap  instance_map_;
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_CLASSREGISTRY_H
