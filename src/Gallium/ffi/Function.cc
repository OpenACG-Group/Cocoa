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

#include "Core/Errors.h"
#include "Gallium/ffi/Function.h"
#include "Gallium/RuntimeBase.h"
GALLIUM_FFI_NS_BEGIN

using v8::External;

namespace {

struct NormalFuncCallData
{
    DirectCallable callable;
};

void ffi_normal_func_call(const FunctionCallbackInfo<Value>& info)
{
    Isolate *isolate = info.GetIsolate();
    if (!info.Data()->IsExternal())
    {
        isolate->ThrowError("InternalError: failed to call the wrapped function, broken closure");
        return;
    }
    auto *call_data = static_cast<NormalFuncCallData*>(info.Data().As<External>()->Value());
    CHECK(call_data && call_data->callable);
    call_data->callable(info);
}

} // namespace anonymous

Local<FunctionTemplate> WrapFunction(Isolate *isolate, DirectCallable callable,
                                     v8::SideEffectType side_effect_type)
{
    CHECK(isolate);
    CHECK(callable);

    auto *call_data = new NormalFuncCallData{ std::move(callable) };
    RuntimeBase *runtime = RuntimeBase::FromIsolate(isolate);
    runtime->AddExternalCallback(
            RuntimeBase::ExternalCallbackType::kAfterDisposeCleanup,
            [call_data]() {
                delete call_data;
                return RuntimeBase::ExternalCallbackAfterCall::kRemove;
            }
    );

    return FunctionTemplate::New(
            /* isolate= */ isolate,
            /* callback= */ ffi_normal_func_call,
            /* data= */ External::New(isolate, call_data),
            /* signature= */ {},
            /* length= */ 0,
            /* behavior= */ v8::ConstructorBehavior::kThrow,
            /* side_effect_type= */ side_effect_type,
            /* c_function= */ nullptr,
            /* instance_type= */ 0,
            /* allowed_receiver_instance_type_range_start= */0,
            /* allowed_receiver_instance_type_range_end= */ 0
    );
}

namespace fn {

bool CheckThisDisposeState(JSObject *this_, bool skip_disposing_check, bool skip_disposed_check)
{
    Isolate *isolate = Isolate::GetCurrent();

    JSObject::DisposeState state = this_->GetDisposeState();
    if (!skip_disposing_check && state == JSObject::DisposeState::kDisposing)
    {
        isolate->ThrowError("Object is disposing, method call or property access is not allowed");
        return false;
    }
    if (!skip_disposed_check && state == JSObject::DisposeState::kDisposed)
    {
        isolate->ThrowError("Object is disposed, method call or property access is not allowed");
        return false;
    }
    return true;
}

void CallNotConstructible(const FunctionCallbackInfo<Value>& info)
{
    Isolate *isolate = info.GetIsolate();
    if (!info.IsConstructCall()) {
        isolate->ThrowError("The function must be called as a constructor");
        return;
    }
    isolate->ThrowError("Object is not constructible from JavaScript");
}

}

Local<FunctionTemplate> WrapNotConstructibleConstructor(Isolate *isolate)
{
    return FunctionTemplate::New(
            isolate,
            fn::CallNotConstructible,
            {},
            {},
            0,
            v8::ConstructorBehavior::kAllow,
            v8::SideEffectType::kHasSideEffect,
            nullptr,
            0
    );
}

GALLIUM_FFI_NS_END
