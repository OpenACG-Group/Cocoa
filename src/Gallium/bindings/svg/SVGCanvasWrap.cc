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

#include "include/core/SkRect.h"
#include "include/svg/SkSVGCanvas.h"

#include "fmt/format.h"

#include "Gallium/bindings/svg/Exports.h"
#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

bool SVGJSWStreamImpl::write(const void *buffer, size_t size)
{
    v8::HandleScope scope(isolate_);

    v8::Local<v8::Context> ctx = isolate_->GetCurrentContext();
    v8::Local<v8::Function> func = func_.Get(isolate_);

    auto [scoped_obj, scoped_obj_ptr] = CallbackScopedBuffer::MakeScoped(
            reinterpret_cast<uint8_t*>(const_cast<void*>(buffer)), size, true);

    CallbackScopedBuffer::ScopeGuard bufscope(scoped_obj, scoped_obj_ptr);

    v8::Local<v8::Value> args[] = { scoped_obj };
    if (!func->Call(ctx, v8::Null(isolate_), 1, args).IsEmpty())
        return false;

    bytes_written_ += size;
    return true;
}

void SVGJSWStreamImpl::flush()
{
}

size_t SVGJSWStreamImpl::bytesWritten() const
{
    return bytes_written_;
}

v8::Local<v8::Value> SVGCanvasWrap::Make(v8::Local<v8::Value> bounds,
                                         v8::Local<v8::Value> writer,
                                         uint32_t flags)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    SkRect rect = glamor_wrap::ExtractCkRect(isolate, bounds);
    if (!writer->IsFunction())
        g_throw(TypeError, "Argument `writer` must be a function");

    auto stream = std::make_unique<SVGJSWStreamImpl>(isolate, writer.As<v8::Function>());
    std::unique_ptr<SkCanvas> canvas = SkSVGCanvas::Make(rect, stream.get(), flags);

    return binder::NewObject<SVGCanvasWrap>(
            isolate, std::move(canvas), std::move(stream));
}

void SVGCanvasWrap::finish()
{
    glamor_wrap::CkCanvas::InvalidateCanvasRef();
    canvas_.reset();
    stream_.reset();
}

GALLIUM_BINDINGS_SVG_NS_END
