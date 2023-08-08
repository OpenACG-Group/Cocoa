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

#include "Gallium/Gallium.h"
#include "Gallium/binder/Class.h"

GALLIUM_BINDER_NS_BEGIN

namespace detail {

static std::string pointer_str(void const *ptr)
{
    std::string buf(sizeof(void *) * 2 + 3, 0); // +3 for 0x and \0 terminator
    int const len = snprintf(&buf[0], buf.size(), "%p", ptr);
    buf.resize(len < 0 ? 0 : len);
    return buf;
}

/////////////////////////////////////////////////////////////////////////////
//
// ClassInfo
//
ClassInfo::ClassInfo(type_info const& type, type_info const& traits)
        : type(type), traits(traits)
{
}

std::string ClassInfo::class_name() const
{
    return "cocoa::koi::binder::Class<" + std::string(type.name()) + ", " + std::string(traits.name()) + ">";
}

/////////////////////////////////////////////////////////////////////////////
//
// ObjectRegistry
//
template<typename Traits>
ObjectRegistry<Traits>::ObjectRegistry(v8::Isolate *isolate, type_info const& type, dtor_function&& dtor)
        : ClassInfo(type, type_id<Traits>()), isolate_(isolate),
          ctor_() // no wrapped class constructor available by default
        , dtor_(std::move(dtor)), auto_wrap_objects_(false)
{
    v8::HandleScope scope(isolate_);

    v8::Local<v8::FunctionTemplate> func = v8::FunctionTemplate::New(isolate_);
    v8::Local<v8::FunctionTemplate> js_func
        = v8::FunctionTemplate::New(isolate_, [](v8::FunctionCallbackInfo<v8::Value> const& args) {
                v8::Isolate *isolate = args.GetIsolate();
                ObjectRegistry *this_ = external_data::get<ObjectRegistry *>(args.Data());
                try
                {
                    return args.GetReturnValue().Set(this_->wrap_object(args));
                }
                catch (const JSException& ex)
                {
                    args.GetReturnValue().Set(JSException::TakeOver(ex));
                }
                catch (std::exception const& ex)
                {
                    args.GetReturnValue().Set(throw_(isolate, ex.what()));
                }
            }, external_data::set(isolate, this));

    func_.Reset(isolate, func);
    js_func_.Reset(isolate, js_func);

    // each JavaScript instance has 3 internal fields:
    //  0 - pointer to a wrapped C++ object
    //  1 - pointer to this ObjectRegistry
    //  3 - pointer to the object descriptor
    func->InstanceTemplate()->SetInternalFieldCount(kInternalFieldsCount);
    func->Inherit(js_func);
}

template<typename Traits>
ObjectRegistry<Traits>::~ObjectRegistry()
{
    remove_objects();
}

template<typename Traits>
void ObjectRegistry<Traits>::add_base(ObjectRegistry& info, cast_function cast)
{
    auto it = std::find_if(bases_.begin(), bases_.end(),
                           [&info](base_class_info const& base) { return &base.info == &info; });
    if (it != bases_.end())
    {
        //assert(false && "duplicated inheritance");
        throw std::runtime_error(class_name()
                                 + " is already inherited from " + info.class_name());
    }
    bases_.emplace_back(info, cast);
    info.derivatives_.emplace_back(this);
}

template<typename Traits>
bool ObjectRegistry<Traits>::cast(pointer_type& ptr, type_info const& type) const
{
    if (this->type == type || !ptr)
    {
        return true;
    }

    // fast way - search a direct parent
    for (base_class_info const& base : bases_)
    {
        if (base.info.type == type)
        {
            ptr = base.cast(ptr);
            return true;
        }
    }

    // slower way - walk on hierarhy
    for (base_class_info const& base : bases_)
    {
        pointer_type p = base.cast(ptr);
        if (base.info.cast(p, type))
        {
            ptr = p;
            return true;
        }
    }
    return false;
}

template<typename Traits>
void ObjectRegistry<Traits>::remove_object(object_id const& obj)
{
    auto it = objects_.find(Traits::key(obj));
    CHECK(it != objects_.end() && "no object");
    if (it != objects_.end())
    {
        v8::HandleScope scope(isolate_);
        reset_object(it->first, it->second);
        objects_.erase(it);
    }
}

template<typename Traits>
void ObjectRegistry<Traits>::remove_objects()
{
    v8::HandleScope scope(isolate_);
    for (auto& object_wrapped : objects_)
    {
        reset_object(object_wrapped.first, object_wrapped.second);
    }
    objects_.clear();
}

template<typename Traits>
typename ObjectRegistry<Traits>::pointer_type
ObjectRegistry<Traits>::find_object(object_id id, type_info const& type) const
{
    auto it = objects_.find(Traits::key(id));
    if (it != objects_.end())
    {
        pointer_type ptr = it->first;
        if (cast(ptr, type))
        {
            return ptr;
        }
    }
    return nullptr;
}

template<typename Traits>
v8::Local<v8::Object> ObjectRegistry<Traits>::find_v8_object(pointer_type const& ptr) const
{
    auto it = objects_.find(ptr);
    if (it != objects_.end())
    {
        return to_local(isolate_, it->second.pobj);
    }

    v8::Local<v8::Object> result;
    for (auto const info : derivatives_)
    {
        result = info->find_v8_object(ptr);
        if (!result.IsEmpty()) break;
    }
    return result;
}

template<typename Traits>
v8::Local<v8::Object>
ObjectRegistry<Traits>::wrap_object(pointer_type const& object,
                                    bindings::ExportableObjectBase::Descriptor *descriptor,
                                    bool call_dtor)
{
    auto it = objects_.find(object);
    if (it != objects_.end())
    {
        //assert(false && "duplicate object");
        throw std::runtime_error(class_name()
                                 + " duplicate object " + pointer_str(Traits::pointer_id(object)));
    }

    v8::EscapableHandleScope scope(isolate_);

    v8::Local<v8::Context> context = isolate_->GetCurrentContext();
    v8::Local<v8::Object> obj = class_function_template()
            ->GetFunction(context).ToLocalChecked()->NewInstance(context).ToLocalChecked();

    obj->SetAlignedPointerInInternalField(kObjectPtr_InternalFields, Traits::pointer_id(object));
    obj->SetAlignedPointerInInternalField(kObjectRegistryPtr_InternalFields, this);
    obj->SetAlignedPointerInInternalField(kObjectDescriptorPtr_InternalFields, descriptor);

    v8::Global<v8::Object> pobj(isolate_, obj);
    pobj.SetWeak(this, [](v8::WeakCallbackInfo<ObjectRegistry> const& data) {
        object_id object = data.GetInternalField(0);
        auto *this_ = static_cast<ObjectRegistry *>(data.GetInternalField(1));
        this_->remove_object(object);
    }, v8::WeakCallbackType::kInternalFields);
    objects_.emplace(object, WrappedObject{std::move(pobj), call_dtor});

    // Also store a weak reference in the object itself
    descriptor->SetObjectWeakReference(isolate_, obj);

    return scope.Escape(obj);
}

template<typename Traits>
v8::Local<v8::Object> ObjectRegistry<Traits>::wrap_object(v8::FunctionCallbackInfo<v8::Value> const& args)
{
    if (!ctor_)
    {
        //assert(false && "create not allowed");
        throw std::runtime_error(class_name() + " has no constructor");
    }

    auto pair = ctor_(args);
    return wrap_object(pair.first, pair.second, true);
}

template<typename Traits>
typename ObjectRegistry<Traits>::pointer_type
ObjectRegistry<Traits>::unwrap_object(v8::Local<v8::Value> value)
{
    v8::HandleScope scope(isolate_);

    while (value->IsObject())
    {
        v8::Local<v8::Object> obj = value.As<v8::Object>();
        if (obj->InternalFieldCount() == kInternalFieldsCount)
        {
            object_id id = obj->GetAlignedPointerFromInternalField(kObjectPtr_InternalFields);
            if (id)
            {
                auto registry = static_cast<ObjectRegistry *>(
                        obj->GetAlignedPointerFromInternalField(kObjectRegistryPtr_InternalFields));
                if (registry)
                {
                    pointer_type ptr = registry->find_object(id, type);
                    if (ptr)
                        return ptr;
                }
            }
        }
        value = obj->GetPrototype();
    }
    return nullptr;
}

template<typename Traits>
void ObjectRegistry<Traits>::reset_object(pointer_type const& object, WrappedObject& wrapped)
{
    if (wrapped.call_dtor)
    {
        dtor_(isolate_, object);
    }
    wrapped.pobj.Reset();
}

/////////////////////////////////////////////////////////////////////////////
//
// classes
//
template<typename Traits>
ObjectRegistry<Traits>& Classes::add(v8::Isolate *isolate, type_info const& type,
                                     typename ObjectRegistry<Traits>::dtor_function&& dtor)
{
    Classes *info = instance(operation::add, isolate);
    auto it = info->find(type);
    if (it != info->classes_.end())
    {
        //assert(false && "class already registred");
        throw std::runtime_error((*it)->class_name()
                                 + " is already exist in isolate " + pointer_str(isolate));
    }
    info->classes_.emplace_back(new ObjectRegistry<Traits>(isolate, type, std::move(dtor)));
    return *static_cast<ObjectRegistry<Traits> *>(info->classes_.back().get());
}

template<typename Traits>
void Classes::remove(v8::Isolate *isolate, type_info const& type)
{
    Classes *info = instance(operation::get, isolate);
    if (info)
    {
        auto it = info->find(type);
        if (it != info->classes_.end())
        {
            type_info const& traits = type_id<Traits>();
            if ((*it)->traits != traits)
            {
                throw std::runtime_error((*it)->class_name()
                                         + " is already registered in isolate "
                                         + pointer_str(isolate) + " before of "
                                         + ClassInfo(type, traits).class_name());
            }
            info->classes_.erase(it);
            if (info->classes_.empty())
            {
                instance(operation::remove, isolate);
            }
        }
    }
}

template<typename Traits>
ObjectRegistry<Traits>& Classes::find(v8::Isolate *isolate, type_info const& type)
{
    Classes *info = instance(operation::get, isolate);
    type_info const& traits = type_id<Traits>();
    if (info)
    {
        auto it = info->find(type);
        if (it != info->classes_.end())
        {
            if ((*it)->traits != traits)
            {
                throw std::runtime_error((*it)->class_name()
                                         + " is already registered in isolate "
                                         + pointer_str(isolate) + " before of "
                                         + ClassInfo(type, traits).class_name());
            }
            return *static_cast<ObjectRegistry<Traits> *>(it->get());
        }
    }
    //assert(false && "class not registered");
    throw std::runtime_error(ClassInfo(type, traits).class_name()
                             + " is not registered in isolate " + pointer_str(isolate));
}

void Classes::remove_all(v8::Isolate *isolate)
{
    instance(operation::remove, isolate);
}

Classes::classes_info::iterator Classes::find(type_info const& type)
{
    return std::find_if(classes_.begin(), classes_.end(),
                        [&type](classes_info::value_type const& info) {
                            return info->type == type;
                        });
}

Classes *Classes::instance(operation op, v8::Isolate *isolate)
{
    static std::unordered_map<v8::Isolate *, Classes> instances;
    switch (op)
    {
    case operation::get:
    {
        auto it = instances.find(isolate);
        return it != instances.end() ? &it->second : nullptr;
    }
    case operation::add:
        return &instances[isolate];
    case operation::remove:
        instances.erase(isolate);
    default:
        return nullptr;
    }
}

template
class ObjectRegistry<raw_ptr_traits>;

template
class ObjectRegistry<shared_ptr_traits>;

template
ObjectRegistry<raw_ptr_traits>& Classes::add<raw_ptr_traits>(v8::Isolate *isolate,
                                                             type_info const& type,
                                                             ObjectRegistry<raw_ptr_traits>::dtor_function&& dtor);

template
void Classes::remove<raw_ptr_traits>(v8::Isolate *isolate, type_info const& type);

template
ObjectRegistry<raw_ptr_traits>& Classes::find<raw_ptr_traits>(v8::Isolate *isolate,
                                                              type_info const& type);

template
ObjectRegistry<shared_ptr_traits>& Classes::add<shared_ptr_traits>(v8::Isolate *isolate,
                                                                   type_info const& type,
                                                                   ObjectRegistry<shared_ptr_traits>::dtor_function&& dtor);

template
void Classes::remove<shared_ptr_traits>(v8::Isolate *isolate, type_info const& type);

template
ObjectRegistry<shared_ptr_traits>& Classes::find<shared_ptr_traits>(v8::Isolate *isolate,
                                                                    type_info const& type);

} // namespace detail

void Cleanup(v8::Isolate* isolate)
{
    detail::Classes::remove_all(isolate);
    detail::external_data::destroy_all(isolate);
}

GALLIUM_BINDER_NS_END
