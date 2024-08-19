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

#include "Gallium/bindings/renderer/PictureRecorder.h"
#include "Gallium/bindings/renderer/Canvas.h"
#include "Gallium/bindings/renderer/Picture.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> PictureRecorder::beginRecording(const RectAdapter& bounds)
{
    if (recorder_.getRecordingCanvas())
        return ffi::Fail(ffi::kErr, "an existing recording has not finished yet");

    SkCanvas *sk_canvas = recorder_.beginRecording(*bounds);
    CHECK(sk_canvas);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto canvas = ffi::JSObject::New<Canvas>(isolate, sk_canvas, GetThisHandle(isolate));
    CHECK(!canvas.IsEmpty() && canvas->IsObject());
    recording_canvas_.Reset(isolate, canvas);

    return canvas;
}

ffi::RetLocal<v8::Value> PictureRecorder::getRecordingCanvas()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (recording_canvas_.IsEmpty())
        return v8::Null(isolate);
    return recording_canvas_.Get(isolate);
}

ffi::RetLocal<v8::Value> PictureRecorder::finishRecordingAsPicture()
{
    if (!recorder_.getRecordingCanvas())
        return ffi::Fail(ffi::kErr, "no existing recording");
    CHECK(!recording_canvas_.IsEmpty());

    sk_sp<SkPicture> sk_pict = recorder_.finishRecordingAsPicture();
    CHECK(sk_pict);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Canvas *canvas = ffi::JSObject::Unwrap<Canvas>(isolate, recording_canvas_.Get(isolate));
    // To notify the canvas that it becomes invalid.
    // Possible user JavaScript execution (registered event listeners).
    canvas->OnParentDispose();

    return ffi::JSObject::New<Picture>(isolate, sk_pict);
}

ffi::RetLocal<v8::Value> PictureRecorder::finishRecordingAsPictureWithCull(const RectAdapter& cull)
{
    if (!recorder_.getRecordingCanvas())
        return ffi::Fail(ffi::kErr, "no existing recording");
    CHECK(!recording_canvas_.IsEmpty());

    sk_sp<SkPicture> sk_pict = recorder_.finishRecordingAsPictureWithCull(*cull);
    CHECK(sk_pict);

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Canvas *canvas = ffi::JSObject::Unwrap<Canvas>(isolate, recording_canvas_.Get(isolate));
    // To notify the canvas that it becomes invalid.
    // Possible user JavaScript execution (registered event listeners).
    canvas->OnParentDispose();

    return ffi::JSObject::New<Picture>(isolate, sk_pict);
}

GALLIUM_BINDINGS_RENDERER_NS_END
