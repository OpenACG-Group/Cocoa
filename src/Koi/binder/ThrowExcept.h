#ifndef COCOA_THROWEXCEPT_H
#define COCOA_THROWEXCEPT_H

#include <string>

#include "include/v8.h"
#include "Koi/binder/Utility.h"
#include "Koi/binder/Convert.h"
KOI_BINDER_NS_BEGIN

v8::Local<v8::Value> throw_(v8::Isolate* isolate,
                            std::string_view str);

v8::Local<v8::Value> throw_(v8::Isolate* isolate,
                            std::string_view str,
                            v8::Local<v8::Value>(*builder)(v8::Local<v8::String>));

class JSException : public std::runtime_error
{
public:
    ~JSException() override = default;

    enum class Category
    {
        kError,
        kRangeError,
        kTypeError,
        kReferenceError,
        kSyntaxError,
        kWasmCompileError,
        kWasmLinkError,
        kWasmRuntimeError
    };

    /**
     * Throw a native C++ exception, which will be caught by binder automatically
     * if current function is called by binder from JavaScript.
     * Binder will convert it to the corresponding JavaScript exception by `TakeOver()`
     * and rethrow the converted exception after catching a JSException (C++ native exception).
     */
    koi_noreturn static void Throw(Category category, const std::string& what,
                                   v8::Isolate *isolate = nullptr);

    static v8::Local<v8::Value> TakeOver(const JSException& except);

    koi_nodiscard inline Category getCategory() const {
        return category_;
    }
    koi_nodiscard inline v8::Isolate *getIsolate() const {
        return isolate_;
    }
    koi_nodiscard v8::Local<v8::Value> asException() const;

private:
    JSException(v8::Isolate *isolate, const std::string& what, Category category)
            : std::runtime_error(what)
            , isolate_(isolate)
            , category_(category) {}

    v8::Isolate *isolate_;
    Category     category_;
};

using ExceptT = JSException::Category;

KOI_BINDER_NS_END
#endif //COCOA_THROWEXCEPT_H
