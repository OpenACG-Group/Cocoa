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

#ifndef COCOA_GALLIUM_FFI_DEFINE_CLASS_H
#define COCOA_GALLIUM_FFI_DEFINE_CLASS_H

#include "include/v8-template.h"

#include "Gallium/Gallium.h"
#include "Gallium/ffi/Function.h"
#include "Gallium/ffi/DataTypes.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/ClassRegistry.h"
GALLIUM_FFI_NS_BEGIN

using v8::SideEffectType;
using v8::Function;

template<typename T, typename Parent = void>
class DefineClass
{
public:
    static_assert(std::is_base_of<JSObject, T>::value, "`T` must be derived from JSObject");
    static_assert(std::is_void<Parent>::value || std::is_base_of<Parent, T>::value, "Invalid `Parent` class");

    explicit DefineClass(Isolate *isolate);

    // Finalize the definition of class and register the class.
    // Returns the JS constructor's FunctionTemplate that can be exported
    // directly into JS.
    Local<FunctionTemplate> Finalize();
    Local<Value> Finalize(Local<Context> context);

    // Set a constructor. JS constructor calls will be forwarded to
    // the registered C++ constructor. A class can only have a single
    // constructor (override is not allowed). But resetting the constructor
    // before adding any methods or properties is allowed.
    template<typename...ArgsT>
    DefineClass& Constructor(bool no_side_effect = true);

    // Reset the deleter. Deleter will be called when the instance of
    // the class can be destroyed (for example, garbage collected).
    DefineClass& OverrideDeleter(ClassMetadata::Deleter func);

    // Reset the inheritance info, which has been set through the
    // template argument `Parent` previously.
    template<typename R>
    DefineClass& Inherit();

    template<typename RetT, typename...ArgsT>
    DefineClass& MethodPure(const char *name, Return<RetT>(T::*member_func)(ArgsT...),
                            bool skip_disposing_check = false,
                            bool skip_disposed_check = false);

    template<typename RetT, typename...ArgsT>
    DefineClass& Method(const char *name, Return<RetT>(T::*member_func)(ArgsT...),
                        bool skip_disposing_check = false,
                        bool skip_disposed_check = false);

    template<typename RetT, typename...ArgsT>
    DefineClass& MethodStaticPure(const char *name, Return<RetT>(*func_addr)(ArgsT...));

    template<typename RetT, typename...ArgsT>
    DefineClass& MethodStatic(const char *name, Return<RetT>(*func_addr)(ArgsT...));

    template<typename PropT>
    DefineClass& Property(const char *name,
                          Return<PropT>(T::*getter)(),
                          Return<void>(T::*setter)(PropT),
                          bool skip_disposing_check = false,
                          bool skip_disposed_check = false);

private:
    Local<FunctionTemplate> GetCtor();

    Isolate                     *isolate_;
    ClassRegistry               *registry_;
    bool                         ctor_locked_;
    Local<FunctionTemplate>      ctor_template_;
    ClassTypeInfo                class_type_info_;
    std::optional<ClassTypeInfo> parent_type_info_;
    ClassMetadata::Deleter       deleter_;
};

namespace fn {

template<typename T>
void InstanceGenericDeleter(void *ptr)
{
    CHECK(ptr && "Null-pointer for instance deleter");
    ::delete static_cast<T*>(ptr);
}

} // namespace fn

template<typename T, typename Parent>
DefineClass<T, Parent>::DefineClass(Isolate *isolate)
        : isolate_(isolate)
        , registry_(ClassRegistry::Get(isolate))
        , ctor_locked_(false)
        , class_type_info_(ClassTypeInfo::Get<T>())
        , deleter_(fn::InstanceGenericDeleter<T>)
{
    if constexpr (!std::is_void<Parent>::value) {
        parent_type_info_ = ClassTypeInfo::Get<Parent>();
    }
}

template<typename T, typename P>
Local<FunctionTemplate> DefineClass<T, P>::GetCtor()
{
    if (ctor_template_.IsEmpty())
    {
        // If is no constructor that is implicitly set,
        // we wrap a not-constructible constructor as a placeholder.
        // That constructor will throw an exception when called.
        ctor_template_ = WrapNotConstructibleConstructor(isolate_);
        ctor_locked_ = true;
    }
    return ctor_template_;
}

template<typename T, typename P>
DefineClass<T, P>& DefineClass<T, P>::OverrideDeleter(ClassMetadata::Deleter func)
{
    deleter_ = func;
    return *this;
}

template<typename T, typename P>
template<typename R>
DefineClass<T, P>& DefineClass<T, P>::Inherit()
{
    // The previous inheritance info set by template argument `P`
    // will be overriden.
    parent_type_info_ = ClassTypeInfo::Get<R>();
    return *this;
}

template<typename T, typename Parent>
Local<FunctionTemplate> DefineClass<T, Parent>::Finalize()
{
    v8::HandleScope handle_scope(isolate_);

    Local<FunctionTemplate> ctor_template = GetCtor();
    ClassMetadata::ToParentCastF to_parent_cast = nullptr;
    if (parent_type_info_)
    {
        to_parent_cast = +[](void *from) -> void* {
            return static_cast<Parent*>(static_cast<T*>(from));
        };
    }
    registry_->AddClass(class_type_info_, T::kJSObjectAttrs, parent_type_info_,
                        to_parent_cast, ctor_template, deleter_);

    return ctor_template;
}

template<typename T, typename Parent>
Local<Value> DefineClass<T, Parent>::Finalize(Local<Context> context)
{
    return Finalize()->GetFunction(context).ToLocalChecked();
}

template<typename T, typename Parent>
template<typename...ArgsT>
DefineClass<T, Parent>& DefineClass<T, Parent>::Constructor(bool no_side_effect)
{
    CHECK(!ctor_locked_ && "Locked constructor cannot be reset");
    ctor_template_ = WrapConstructor<T, ArgsT...>(
            isolate_, no_side_effect ? SideEffectType::kHasNoSideEffect : SideEffectType::kHasSideEffect);
    return *this;
}

template<typename T, typename Parent>
template<typename RetT, typename...ArgsT>
DefineClass<T, Parent>&
DefineClass<T, Parent>::MethodPure(const char *name, Return<RetT>(T::*member_func)(ArgsT...),
                                   bool skip_disposing_check,
                                   bool skip_disposed_check)
{
    auto ft = WrapMemberFunction(isolate_, SideEffectType::kHasNoSideEffect, member_func,
                                 skip_disposing_check, skip_disposed_check);
    GetCtor()->PrototypeTemplate()->Set(isolate_, name, ft, v8::PropertyAttribute::ReadOnly);
    return *this;
}

template<typename T, typename Parent>
template<typename RetT, typename...ArgsT>
DefineClass<T, Parent>&
DefineClass<T, Parent>::Method(const char *name, Return<RetT>(T::*member_func)(ArgsT...),
                               bool skip_disposing_check,
                               bool skip_disposed_check)
{
    auto ft = WrapMemberFunction(isolate_, SideEffectType::kHasSideEffect, member_func,
                                 skip_disposing_check, skip_disposed_check);
    GetCtor()->PrototypeTemplate()->Set(isolate_, name, ft, v8::PropertyAttribute::ReadOnly);
    return *this;
}

template<typename T, typename Parent>
template<typename RetT, typename...ArgsT>
DefineClass<T, Parent>&
DefineClass<T, Parent>::MethodStaticPure(const char *name, Return<RetT>(*func_addr)(ArgsT...))
{
    auto ft = WrapFunctionAddress(isolate_, SideEffectType::kHasNoSideEffect, func_addr);
    GetCtor()->Set(isolate_, name, ft, v8::PropertyAttribute::ReadOnly);
    return *this;
}

template<typename T, typename Parent>
template<typename RetT, typename...ArgsT>
DefineClass<T, Parent>&
DefineClass<T, Parent>::MethodStatic(const char *name, Return<RetT>(*func_addr)(ArgsT...))
{
    auto ft = WrapFunctionAddress(isolate_, SideEffectType::kHasSideEffect, func_addr);
    GetCtor()->Set(isolate_, name, ft, v8::PropertyAttribute::ReadOnly);
    return *this;
}

template<typename T, typename P>
template<typename PropT>
DefineClass<T, P>&
DefineClass<T, P>::Property(const char *name,
                            Return<PropT>(T::*getter)(),
                            Return<void>(T::*setter)(PropT),
                            bool skip_disposing_check,
                            bool skip_disposed_check)
{
    WrapPropertyAccessor(
            isolate_,
            String::NewFromUtf8(isolate_, name).ToLocalChecked(),
            GetCtor()->PrototypeTemplate(),
            getter,
            setter,
            v8::SideEffectType::kHasSideEffect,
            v8::SideEffectType::kHasSideEffect,
            skip_disposing_check,
            skip_disposed_check
    );
    return *this;
}

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_DEFINE_CLASS_H
