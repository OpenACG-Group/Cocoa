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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PICTURERECORDER_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PICTURERECORDER_H

#include "include/core/SkPictureRecorder.h"

#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Rect.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Picture;

//! TSDecl: @class PictureRecorder
class PictureRecorder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor()
    PictureRecorder() = default;
    ~PictureRecorder() override = default;

    //! TSDecl: @method beginRecording(bounds: Rect): Canvas
    ffi::RetLocal<v8::Value> beginRecording(const RectAdapter& bounds);

    //! TSDecl: @method getRecordingCanvas(): @union(null, Canvas)
    ffi::RetLocal<v8::Value> getRecordingCanvas();

    //! TSDecl: @method finishRecordingAsPicture(): Picture
    ffi::RetLocal<v8::Value> finishRecordingAsPicture();

    //! TSDecl: @method finishRecordingAsPictureWithCull(cull: Rect): Picture
    ffi::RetLocal<v8::Value> finishRecordingAsPictureWithCull(const RectAdapter& cull);

private:
    SkPictureRecorder recorder_;
    v8::Global<v8::Object> recording_canvas_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PICTURERECORDER_H
