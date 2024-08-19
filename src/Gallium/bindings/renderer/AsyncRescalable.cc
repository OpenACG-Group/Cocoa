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

#include "Gallium/bindings/renderer/ImageAsyncReadResult.h"
#include "Gallium/bindings/renderer/AsyncRescalable.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

std::tuple<AsyncRescaleContext*, v8::Local<v8::Promise>>
AsyncRescaleContext::Create(v8::Isolate *isolate, uint32_t plane_count)
{
    void *addr = ::malloc(sizeof(AsyncRescaleContext) + sizeof(Plane) * plane_count);
    new(addr) AsyncRescaleContext{};
    AsyncRescaleContext *ctx = static_cast<AsyncRescaleContext*>(addr);
    ctx->plane_count = plane_count;

    auto context = isolate->GetCurrentContext();
    auto resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

    ctx->isolate = isolate;
    ctx->resolver.Reset(isolate, resolver);

    return { ctx, resolver->GetPromise() };
}

void AsyncRescaleContext::Delete(AsyncRescaleContext *ctx)
{
    ctx->~AsyncRescaleContext();
    ::free(ctx);
}

void AsyncRescaleContext::Callback(void *ctx, std::unique_ptr<const SkImage::AsyncReadResult> result)
{
    AsyncRescaleContext *async_ctx = static_cast<AsyncRescaleContext*>(ctx);
    async_ctx->PostProcess(std::move(result));
    AsyncRescaleContext::Delete(async_ctx);
}

void AsyncRescaleContext::PostProcess(std::unique_ptr<const SkImage::AsyncReadResult> result)
{
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Promise::Resolver> res = resolver.Get(isolate);
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    if (!result)
    {
        res->Reject(ctx, v8::String::NewFromUtf8Literal(
                isolate, "failed to read and rescale the pixels")).Check();
        return;
    }

    CHECK(plane_count == result->count());
    ImageAsyncReadResult::PlaneVec plane_vec(plane_count);
    for (uint32_t i = 0; i < plane_count; i++)
    {
        plane_vec[i].size = planes[i].size;
        plane_vec[i].bytes_per_pixel = planes[i].bytes_per_pixel;
    }

    v8::Local<v8::Object> obj = ffi::JSObject::New<ImageAsyncReadResult>(
            isolate, std::move(result), std::move(plane_vec));
    res->Resolve(ctx, obj).Check();
}

GALLIUM_BINDINGS_RENDERER_NS_END
