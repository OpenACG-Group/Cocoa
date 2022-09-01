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

#include "uv.h"
#include "fmt/format.h"
#include "Core/EventLoop.h"
#include "Reactor/Reactor.h"
#include "Gallium/bindings/reactor/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_REACTOR_NS_BEGIN

namespace {

struct AsyncCompileClosure
{
    ~AsyncCompileClosure() {
        resolver.Reset();
    }

    uv_work_t work{};
    std::shared_ptr<reactor::GShaderBuilder> builder;
    std::shared_ptr<reactor::GShaderModule> artifact;
    v8::Global<v8::Promise::Resolver> resolver;
    std::string error_info;
};

void on_compilation_task(uv_work_t *work)
{
    auto *closure = reinterpret_cast<AsyncCompileClosure*>(
            uv_handle_get_data(reinterpret_cast<uv_handle_t*>(work)));
    CHECK(closure);
    try {
        closure->artifact = reactor::GShaderModule::Compile(*closure->builder);
    } catch (const std::exception& e) {
        closure->error_info = e.what();
    }
    closure->builder.reset();
}

void after_compilation_task(uv_work_t *work, int status)
{
    auto *closure = reinterpret_cast<AsyncCompileClosure*>(
            uv_handle_get_data(reinterpret_cast<uv_handle_t*>(work)));
    CHECK(closure);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Promise::Resolver> r = closure->resolver.Get(isolate);
    if (!closure->artifact)
    {
        v8::Local<v8::String> msg = binder::to_v8(isolate,
            fmt::format("Failed to compile GShader module: {}", closure->error_info));
        r->Reject(ctx, v8::Exception::Error(msg)).Check();
    }
    else
    {
        v8::Local<v8::Object> artifact = binder::Class<GShaderModuleWrap>::create_object(isolate,
                                                                                         closure->artifact);
        r->Resolve(ctx, artifact).Check();
    }

    delete closure;
}

}

GShaderModuleWrap::GShaderModuleWrap(std::shared_ptr<reactor::GShaderModule> module)
    : module_(std::move(module))
{
}

v8::Local<v8::Object> GShaderModuleWrap::Compile(v8::Local<v8::Object> builder)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    GShaderBuilderWrap *wrap;
    try {
        wrap = binder::Class<GShaderBuilderWrap>::unwrap_object(isolate, builder);
    } catch (const std::exception& e) {
        g_throw(TypeError, "builder must be GShaderBuilder type");
    }
    CHECK(wrap);

    v8::Local<v8::Promise::Resolver> resolver =
            v8::Promise::Resolver::New(ctx).ToLocalChecked();

    auto *closure = new AsyncCompileClosure;
    closure->resolver.Reset(isolate, resolver);
    closure->builder = wrap->builder_;

    uv_handle_set_data(reinterpret_cast<uv_handle_t *>(&closure->work), closure);

    uv_queue_work(EventLoop::Ref().handle(), &closure->work,
                  on_compilation_task, after_compilation_task);

    return resolver->GetPromise();
}

void GShaderModuleWrap::executeMain()
{
    module_->Execute();
}

GALLIUM_BINDINGS_REACTOR_NS_END
