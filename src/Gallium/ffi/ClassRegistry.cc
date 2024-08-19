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

#include "Gallium/ffi/ClassRegistry.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Function.h"
#include "Gallium/ffi/Context.h"
GALLIUM_FFI_NS_BEGIN

ClassRegistry::ClassRegistry(Isolate *isolate)
    : isolate_(isolate)
{
}

ClassRegistry::~ClassRegistry() = default;

ClassRegistry *ClassRegistry::Get(Isolate *isolate)
{
    return TypeContext::FromIsolate(isolate)->GetClassRegistry();
}

void ClassRegistry::AddClass(const ClassTypeInfo& type_info,
                             int attrs,
                             const std::optional<ClassTypeInfo>& parent_type_info,
                             ClassMetadata::ToParentCastF parent_cast_pfn,
                             Local<FunctionTemplate> ctor_func_template,
                             ClassMetadata::Deleter deleter)
{
    CHECK(!ctor_func_template.IsEmpty());
    CHECK(class_info_map_.count(type_info) == 0 && "Duplicated class registration");

    v8::HandleScope handle_scope(isolate_);
    Local<FunctionTemplate> class_func = FunctionTemplate::New(isolate_);
    class_func->InstanceTemplate()->SetInternalFieldCount(ClassMetadata::kFields_count);
    class_func->Inherit(ctor_func_template);

    auto& entry = class_info_map_[type_info];
    entry.class_type_info = type_info;
    entry.attrs = attrs;
    entry.class_registry = this;
    entry.parent_metadata = nullptr;
    entry.parent_cast_pfn = nullptr;
    entry.ctor_func_template.Reset(isolate_, ctor_func_template);
    entry.class_func_template.Reset(isolate_, class_func);
    entry.deleter = deleter;

    if (parent_type_info)
    {
        CHECK(parent_cast_pfn);
        CHECK(class_info_map_.count(*parent_type_info) > 0 && "Parent class not registered");
        entry.parent_metadata = &class_info_map_[*parent_type_info];
        entry.parent_cast_pfn = parent_cast_pfn;

        // Implement the inheritance sematic in JS
        ctor_func_template->Inherit(
                entry.parent_metadata->class_func_template.Get(isolate_));
    }
}

Local<Object> ClassRegistry::WrapObject(const ClassTypeInfo& type_info, JSObject *base_ptr, void *ptr)
{
    CHECK(class_info_map_.count(type_info) > 0 && "Class not registered");

    v8::EscapableHandleScope handle_scope(isolate_);
    auto& entry = class_info_map_[type_info];

    Local<Context> context = isolate_->GetCurrentContext();
    Local<FunctionTemplate> func_temp = entry.class_func_template.Get(isolate_);
    Local<Object> object = func_temp->GetFunction(context).ToLocalChecked()
            ->NewInstance(context).ToLocalChecked();

    object->SetAlignedPointerInInternalField(ClassMetadata::kField_base_ptr, base_ptr);
    object->SetAlignedPointerInInternalField(ClassMetadata::kField_ptr, ptr);

    Global<Object> self_weak(isolate_, object);
    self_weak.SetWeak(this, [](const v8::WeakCallbackInfo<ClassRegistry>& data) {
        auto *base_ptr = static_cast<JSObject*>(
                data.GetInternalField(ClassMetadata::kField_base_ptr));
        CHECK(base_ptr &&
              base_ptr->class_metadata_ &&
              base_ptr->class_metadata_->class_registry);

        auto *instance_ptr = data.GetInternalField(ClassMetadata::kField_ptr);
        CHECK(instance_ptr);

        // Destruct the instance, free the allocated memory, and remove
        // the instance entry that was registered.
        base_ptr->class_metadata_->class_registry->DeleteInstance(base_ptr, instance_ptr);
    }, v8::WeakCallbackType::kInternalFields);

    // Set class metadata for the instance. Every instance of the same class
    // shares the same class metadata pointer.
    base_ptr->class_metadata_ = &entry;
    base_ptr->self_weak_ = std::move(self_weak);

    // Register the instance info. It will be used to delete objects.
    instance_map_.emplace(&entry, ClassMetadata::InstanceInfo{
            .base_ptr = base_ptr,
            .instance_ptr = ptr
    });

    return handle_scope.Escape(object);
}

namespace {

MaybeLocal<Object> find_wrapped_object_in_proto(Local<Object> object)
{
    Local<Object> current = object;
    while (true)
    {
        if (current->InternalFieldCount() == ClassMetadata::kFields_count)
            break;
        Local<Value> proto = current->GetPrototype();
        if (proto->IsNullOrUndefined())
            return {};
        current = proto.As<Object>();
    }
    return current;
}

} // namespace anonymous

JSObject *ClassRegistry::UnwrapObjectBase(Local<v8::Object> object)
{
    Local<Object> current;
    if (!find_wrapped_object_in_proto(object).ToLocal(&current))
        return nullptr;

    return static_cast<JSObject*>(
            current->GetAlignedPointerFromInternalField(ClassMetadata::kField_base_ptr));
}

void *ClassRegistry::UnwrapObject(const ClassTypeInfo& type_info, Local<Object> object)
{
    auto class_type_itr = class_info_map_.find(type_info);
    CHECK(class_type_itr != class_info_map_.end() && "Unregistered class");

    Local<Object> current;
    if (!find_wrapped_object_in_proto(object).ToLocal(&current))
        return nullptr;

    JSObject *base_ptr = static_cast<JSObject*>(
            current->GetAlignedPointerFromInternalField(ClassMetadata::kField_base_ptr));
    void *instance_ptr = current->GetAlignedPointerFromInternalField(ClassMetadata::kField_ptr);
    CHECK(base_ptr && instance_ptr);

    ClassMetadata *cur_metadata = base_ptr->class_metadata_;
    while (cur_metadata && cur_metadata->class_type_info != type_info)
    {
        CHECK(cur_metadata->parent_cast_pfn);
        instance_ptr = cur_metadata->parent_cast_pfn(instance_ptr);
        cur_metadata = cur_metadata->parent_metadata;
    }

    if (!cur_metadata)
        return nullptr;

    return instance_ptr;
}

void ClassRegistry::DeleteInstance(JSObject *base_ptr, void *instance_ptr)
{
    ClassMetadata *metadata = base_ptr->class_metadata_;

    // Instance is destructed here
    if (base_ptr->class_metadata_->deleter)
        base_ptr->class_metadata_->deleter(instance_ptr);

    // Remove the registered instance entry
    bool found = false;
    auto [itr, end] = instance_map_.equal_range(metadata);
    while (itr != end)
    {
        if (itr->second.base_ptr == base_ptr)
        {
            CHECK(itr->second.instance_ptr == instance_ptr);
            instance_map_.erase(itr);
            found = true;
            break;
        }
        itr++;
    }
    CHECK(found);
}

void ClassRegistry::Dispose()
{
    for (auto [metadata, instance_info] : instance_map_)
    {
        if (metadata->deleter)
            metadata->deleter(instance_info.instance_ptr);
    }
    instance_map_.clear();
}

GALLIUM_FFI_NS_END
