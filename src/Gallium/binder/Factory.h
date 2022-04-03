#ifndef COCOA_GALLIUM_BINDER_FACTORY_H
#define COCOA_GALLIUM_BINDER_FACTORY_H

#include <memory>

#include "include/v8.h"
#include "Gallium/Gallium.h"

GALLIUM_BINDER_NS_BEGIN

template<typename T, typename Traits>
struct Factory
{
    using object_pointer_type = typename Traits::template object_pointer_type<T>;

    template<typename ...Args>
    static object_pointer_type create(v8::Isolate *isolate, Args... args)
    {
        object_pointer_type object = Traits::template create<T>(std::forward<Args>(args)...);
        isolate->AdjustAmountOfExternalAllocatedMemory(
                static_cast<int64_t>(Traits::object_size(object)));
        return object;
    }

    static void destroy(v8::Isolate *isolate, object_pointer_type const& object)
    {
        isolate->AdjustAmountOfExternalAllocatedMemory(
                -static_cast<int64_t>(Traits::object_size(object)));
        Traits::destroy(object);
    }
};

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_FACTORY_H
