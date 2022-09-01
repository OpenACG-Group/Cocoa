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

#ifndef COCOA_GALLIUM_BINDER_PTRTRAITS_H
#define COCOA_GALLIUM_BINDER_PTRTRAITS_H

#include <memory>

#include "Gallium/Gallium.h"
GALLIUM_BINDER_NS_BEGIN

template<typename T, typename Enable = void>
struct convert;

struct raw_ptr_traits
{
    using pointer_type = void*;
    using const_pointer_type = const void*;

    template<typename T>
    using object_pointer_type = T*;
    template<typename T>
    using object_const_pointer_type = const T*;

    using object_id = void*;

    static object_id pointer_id(void *ptr)
    { return ptr; }
    static pointer_type key(object_id id)
    { return id; }
    static pointer_type const_pointer_cast(const_pointer_type ptr)
    { return const_cast<void*>(ptr); }
    template<typename T, typename U>
    static T *static_pointer_cast(U *ptr)
    { return static_cast<T*>(ptr); }

    template<typename T>
    using convert_ptr = convert<T*>;

    template<typename T>
    using convert_ref = convert<T&>;

    template<typename T, typename...Args>
    static object_pointer_type<T> create(Args&&... args)
    {
        return new T(std::forward<Args>(args)...);
    }

    template<typename T>
    static object_pointer_type<T> clone(const T& src)
    {
        return new T(src);
    }

    template<typename T>
    static void destroy(const object_pointer_type<T>& ptr)
    {
        delete ptr;
    }

    template<typename T>
    constexpr static size_t object_size(const object_pointer_type<T>&)
    {
        return sizeof(T);
    }
};

struct ref_from_shared_ptr {};

struct shared_ptr_traits
{
    using pointer_type = std::shared_ptr<void>;
    using const_pointer_type = std::shared_ptr<const void>;

    template<typename T>
    using object_pointer_type = std::shared_ptr<T>;
    template<typename T>
    using object_const_pointer_type = std::shared_ptr<const T>;

    using object_id = void*;

    static object_id pointer_id(pointer_type const& ptr)
    { return ptr.get(); }
    static pointer_type key(object_id id)
    {
        // A shared_ptr with an empty deleter
        return {id, [](void*) {}}; // std::shared_ptr<void>(id, [](void*) {})
    }
    static pointer_type const_pointer_cast(const_pointer_type const& ptr)
    { return std::const_pointer_cast<void>(ptr); }
    template<typename T, typename U>
    static std::shared_ptr<T> static_pointer_cast(std::shared_ptr<U> const& ptr)
    { return std::static_pointer_cast<T>(ptr); }

    template<typename T>
    using convert_ptr = convert<std::shared_ptr<T>>;

    template<typename T>
    using convert_ref = convert<T, ref_from_shared_ptr>;

    template<typename T, typename ...Args>
    static object_pointer_type<T> create(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    static object_pointer_type<T> clone(T const& src)
    {
        return std::make_shared<T>(src);
    }

    template<typename T>
    static void destroy(object_pointer_type<T> const&)
    {
        // Do nothing with reference-counted object
    }

    template<typename T>
    static size_t object_size(object_pointer_type<T> const&)
    {
        return sizeof(T);
    }
};

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_PTRTRAITS_H
