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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_CKPICTURERECORDER_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_CKPICTURERECORDER_H

#include "include/core/SkPictureRecorder.h"
#include "include/v8.h"

#include "Gallium/bindings/glamor/TrivialSkiaExportedTypes.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkPictureRecorder
class CkPictureRecorder
{
public:
    //! TSDecl: constructor()
    CkPictureRecorder() = default;
    ~CkPictureRecorder() = default;

    //! TSDecl: function beginRecording(bounds: CkRect): CkCanvas
    v8::Local<v8::Value> beginRecording(v8::Local<v8::Value> bounds);

    //! TSDecl: function getRecordingCanvas(): CkCanvas | null
    v8::Local<v8::Value> getRecordingCanvas();

    //! TSDecl: function finishRecordingAsPicture(): CkPicture | null
    v8::Local<v8::Value> finishRecordingAsPicture();

    //! TSDecl: function finishRecordingAsPictureWithCull(cull: CkRect): CkPicture | null
    v8::Local<v8::Value> finishRecordingAsPictureWithCull(v8::Local<v8::Value> cull);

private:
    SkPictureRecorder recorder_;
    v8::Global<v8::Object> canvas_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_CKPICTURERECORDER_H
