#ifndef COCOA_GALLIUM_BINDINGS_BASE_H
#define COCOA_GALLIUM_BINDINGS_BASE_H

#include <string>
#include <functional>
#include <utility>

#include "Gallium/Gallium.h"
#include "Gallium/binder/Module.h"
#include "Gallium/binder/Factory.h"

GALLIUM_BINDINGS_NS_BEGIN

// NOLINTNEXTLINE
using __voidptr__ = void*;

#ifdef __GNUG__
#define gal_export      __attribute__((visibility("default")))
#else
#define koi_export
#endif

#define gal_clinkage   extern "C"

/**
 * Implement the "hook function" of a dynamic language binding.
 *
 * A dynamic language binding example (hook function, binding class is "ExampleBinding"):
 * @code
 * GALLIUM_BINDING_LOADER_HOOK {
 *   GALLIUM_HOOK_RET(new ExampleBinding());
 * }
 * @endcode
 */
#define GALLIUM_BINDING_LOADER_HOOK     gal_clinkage gal_export __voidptr__ __g_cocoa_hook ()

#define TYPE_PTR(T)                     T*
#define GALLIUM_HOOK_RET(instance)      return static_cast<TYPE_PTR(BindinBase)>(instance)
#undef TYPE_PTR

#define GALLIUM_BINDING_OBJECT                          \
public:                                                 \
    const char *onGetUniqueId() override;               \
    void onGetModule(binder::Module& self) override;    \
    const char **onGetExports() override;               \

class BindingBase
{
public:
    template<typename T>
    using ClassExport = std::unique_ptr<binder::Class<T, binder::raw_ptr_traits>>;

    template<typename T>
    static ClassExport<T> NewClassExport(v8::Isolate *isolate) {
        return std::make_unique<binder::Class<T, binder::raw_ptr_traits>>(isolate);
    }

    BindingBase(std::string name, std::string desc)
        : fName(std::move(name)),
          fDescription(std::move(desc)) {}
    virtual ~BindingBase() = default;

    g_nodiscard inline const std::string& name() const
    { return fName; }
    g_nodiscard inline const std::string& description() const
    { return fDescription; }

    virtual const char *onGetUniqueId() { return nullptr; }

    virtual void onSetInstanceProperties(v8::Local<v8::Object> instance) {}

    /**
     * There are generally two ways to create (construct) a C++ object and wrap
     * it into JavaScript: constructing in JavaScript or constructing in C++.
     *
     * To construct an object in JavaScript way, you need to expose its constructor
     * to JavaScript and then call the constructor in JavaScript directly or by V8
     * indirectly (v8::Object::New). In that case, you should export corresponding
     * `binder::Class<...>` object to JavaScript, and all the parameters of the
     * constructor must be types which can be converted from JavaScript
     * (or `v8::FunctionCallbackInfo<...>`).
     * If we want to construct the object in C++, `Runtime::newObjectFromSynthetic`
     * can be helpful.
     *
     * To construct an object in C++ way, `binder::Class<T>::create_object` should
     * be used. Registering the class (by constructing a corresponding binder::Class<T>
     * object) is required before doing that. In that case, the parameters of the
     * constructor can be any types without any extra restrictions.
     *
     * Anyway, if we want to do in the second way, we must register corresponding
     * classes here. This method will be called once when the module is imported.
     */
    virtual void onRegisterClasses(v8::Isolate *isolate) {}

    /**
     * Override but don't implement this virtual method
     * because the corresponding implementation will be generated automatically.
     */
    virtual void onGetModule(binder::Module& mod) = 0;

    /**
     * Override but don't implement this virtual method
     * because the corresponding implementation will be generated automatically.
     */
    virtual const char **onGetExports() = 0;

private:
    std::string   fName;
    std::string   fDescription;
};

class PreventGCObject
{
public:
    explicit PreventGCObject(v8::Isolate *isolate) : isolate_(isolate) {}
    virtual ~PreventGCObject() = default;

    inline void setGCObjectSelfHandle(v8::Local<v8::Object> self) {
        self_.Reset(isolate_, self);
    }

    inline void markCanBeGarbageCollected() {
        self_.Reset();
    }

private:
    v8::Isolate            *isolate_;
    v8::Global<v8::Object>  self_;
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_BASE_H
