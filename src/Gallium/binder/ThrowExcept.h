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

#ifndef COCOA_GALLIUM_BINDER_THROWEXCEPT_H
#define COCOA_GALLIUM_BINDER_THROWEXCEPT_H

#include <string>

#include "include/v8.h"
#include "Gallium/binder/Utility.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDER_NS_BEGIN

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
    g_noreturn static void Throw(Category category, const std::string& what,
                                   v8::Isolate *isolate = nullptr);

    static v8::Local<v8::Value> TakeOver(const JSException& except);

    g_nodiscard inline Category getCategory() const {
        return category_;
    }
    g_nodiscard inline v8::Isolate *getIsolate() const {
        return isolate_;
    }
    g_nodiscard v8::Local<v8::Value> asException() const;

private:
    JSException(v8::Isolate *isolate, const std::string& what, Category category)
            : std::runtime_error(what)
            , isolate_(isolate)
            , category_(category) {}

    v8::Isolate *isolate_;
    Category     category_;
};

using ExceptT = JSException::Category;

#define g_throw(c, what)        binder::JSException::Throw(binder::ExceptT::k##c, (what))
#define g_throw3(c, what, i)    binder::JSException::Throw(binder::ExceptT::k##c, (what), (i))

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_THROWEXCEPT_H
