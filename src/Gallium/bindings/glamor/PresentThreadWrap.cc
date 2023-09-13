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

#include "Glamor/Glamor.h"
#include "Glamor/PresentThread.h"
#include "Glamor/Display.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> PresentThreadWrap::Start()
{
    auto& gl_global = gl::GlobalScope::Ref();
    if (gl_global.GetPresentThread())
        g_throw(Error, "Present thread has already been started");
    if (!gl_global.StartPresentThread())
        g_throw(Error, "Failed to start present thread");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<PresentThreadWrap>(
            isolate, gl_global.GetPresentThread());
}

void PresentThreadWrap::dispose()
{
    thread_ = nullptr;
    gl::GlobalScope::Ref().DisposePresentThread();
}

void PresentThreadWrap::CheckDisposeOrThrow()
{
    if (!thread_)
        g_throw(Error, "PresentThread has been disposed");
}

v8::Local<v8::Value> PresentThreadWrap::createDisplay()
{
    CheckDisposeOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteTask::Submit<std::shared_ptr<gl::Display>>(
        isolate,
        [] {
            auto *thread_ctx = gl::PresentThread::LocalContext::GetCurrent();
            return gl::Display::Connect(thread_ctx->GetEventLoop(), "");
        },
        [](std::shared_ptr<gl::Display> display) {
            v8::Isolate *isolate = v8::Isolate::GetCurrent();
            return binder::NewObject<DisplayWrap>(isolate, std::move(display));
        }
    );
}

v8::Local<v8::Value> PresentThreadWrap::traceResourcesJSON()
{
    CheckDisposeOrThrow();
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

void PresentThreadWrap::collect()
{
    CheckDisposeOrThrow();
    thread_->GetRemoteDestroyablesCollector()->Collect();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
