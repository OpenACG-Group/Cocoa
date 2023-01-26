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
#include "Gallium/bindings/glamor/CkPictureRecorder.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> CkPictureRecorder::beginRecording(v8::Local<v8::Value> bounds)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkCanvas *canvas = recorder_.beginRecording(ExtractCkRect(isolate, bounds));
    CHECK(canvas);
    auto obj = binder::Class<CkCanvas>::create_object(isolate, canvas);
    canvas_.Reset(isolate, obj);
    return obj;
}

v8::Local<v8::Value> CkPictureRecorder::getRecordingCanvas()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (canvas_.IsEmpty())
        return v8::Null(isolate);
    return canvas_.Get(isolate);
}

v8::Local<v8::Value> CkPictureRecorder::finishRecordingAsPicture()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (canvas_.IsEmpty())
        return v8::Null(isolate);

    sk_sp<SkPicture> pict = recorder_.finishRecordingAsPicture();
    CHECK(pict);
    canvas_.Reset();
    return binder::Class<CkPictureWrap>::create_object(isolate, pict);
}

v8::Local<v8::Value> CkPictureRecorder::finishRecordingAsPictureWithCull(v8::Local<v8::Value> cull)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (canvas_.IsEmpty())
        return v8::Null(isolate);
    sk_sp<SkPicture> pict = recorder_.finishRecordingAsPictureWithCull(
            ExtractCkRect(isolate, cull));
    CHECK(pict);
    canvas_.Reset();
    return binder::Class<CkPictureWrap>::create_object(isolate, pict);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
