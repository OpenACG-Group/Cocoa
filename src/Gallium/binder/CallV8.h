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

#ifndef COCOA_GALLIUM_BINDER_CALLV8_H
#define COCOA_GALLIUM_BINDER_CALLV8_H

#include "Gallium/Gallium.h"
#include "Gallium/binder/Convert.h"

GALLIUM_BINDER_NS_BEGIN

template<typename ...Args>
v8::Local<v8::Value> Invoke(v8::Isolate *isolate, v8::Local<v8::Function> func,
                            v8::Local<v8::Value> recv, Args&& ... args)
{
    v8::EscapableHandleScope scope(isolate);

    int const arg_count = sizeof...(Args);
    // +1 to allocate array for arg_count == 0
    v8::Local<v8::Value> v8_args[arg_count + 1] = {
            to_v8(isolate, std::forward<Args>(args))...
    };

    v8::Local<v8::Value> result;
    bool const is_empty_result = func->Call(isolate->GetCurrentContext(),
                                            recv,
                                            arg_count,
                                            v8_args).ToLocal(&result);
    (void) is_empty_result;
    return scope.Escape(result);
}

template<typename...ArgsT>
v8::Local<v8::Value> InvokeMethod(v8::Isolate *isolate, v8::Local<v8::Object> object,
                                  const std::string& method, ArgsT&&... args)
{
    v8::EscapableHandleScope scope(isolate);
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    v8::Local<v8::Value> member = object->Get(context, to_v8(isolate, method))
                                  .FromMaybe(v8::Local<v8::Value>());
    if (!member->IsFunction())
        return {};
    v8::Local<v8::Value> ret = Invoke(isolate, member.As<v8::Function>(),
                                      object, std::forward<ArgsT>(args)...);
    return scope.Escape(ret);
}

GALLIUM_BINDER_NS_END
#endif //COCOA_GALLIUM_BINDER_CALLV8_H
