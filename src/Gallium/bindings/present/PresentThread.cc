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

#include "Glamor/MaybeGpuObject.h"

#include "Gallium/bindings/present/PresentThread.h"
#include "Gallium/bindings/present/Promisify.h"
#include "Gallium/bindings/present/Display.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

ffi::RetLocal<v8::Value> PresentThread::Start()
{
    gl::GlobalScope& gl_global = gl::GlobalScope::Ref();
    if (gl_global.GetPresentThread())
        return ffi::Fail(ffi::kErr, "present thread has already been started");
    if (!gl_global.StartPresentThread())
        return ffi::Fail(ffi::kErr, "failed to start present thread");

    return ffi::JSObject::New<PresentThread>(v8::Isolate::GetCurrent(), gl_global.GetPresentThread());
}

ffi::Ret<void> PresentThread::dispose()
{
    thread_ = nullptr;
    gl::GlobalScope::Ref().DisposePresentThread();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::Ret<void> PresentThread::collect()
{
    thread_->GetRemoteDestroyablesCollector()->Collect();
    return {};
}

ffi::RetLocal<v8::Value> PresentThread::traceResourcesJSON()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteTask::Submit<std::string>(
        isolate,
        [] {
            auto *thread_ctx = gl::PresentThread::LocalContext::GetCurrent();
            return thread_ctx->TraceResourcesJSON();
        },
        [](const std::string& json_string) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            return v8::String::NewFromUtf8(isolate, json_string.c_str())
                   .ToLocalChecked();
        }
    );
}

ffi::RetLocal<v8::Value> PresentThread::createDisplay()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteTask::Submit<std::shared_ptr<gl::Display>>(
        isolate,
        [] {
            auto *thread_ctx = gl::PresentThread::LocalContext::GetCurrent();
            auto display = gl::Display::Connect(thread_ctx->GetEventLoop(), "");
            if (!display)
                throw std::runtime_error("failed to connect to and initialize a Display");
            return display;
        },
        [](std::shared_ptr<gl::Display> display) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            return ffi::JSObject::New<Display>(isolate, std::move(display));
        }
    );
}

GALLIUM_BINDINGS_PRESENT_NS_END
