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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_SURFACE_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_SURFACE_H

#include "Glamor/Surface.h"

#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/bindings/present/Types.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

class Cursor;
class Monitor;

//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) Surface
class Surface : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    Surface(v8::Global<v8::Object> display, std::shared_ptr<gl::Surface> surface);
    ~Surface() override = default;

    //! TSDecl: @property @readonly dimensions: @tuple(i32, i32)
    ffi::RetLocal<v8::Value> getDimensions() {
        using Tuple = std::tuple<int32_t, int32_t>;
        return ffi::Cast<Tuple>::ToChecked(v8::Isolate::GetCurrent(),
                                           {dimensions_.fWidth, dimensions_.fHeight});
    }

    //! TSDecl: @property @readonly display: Display
    ffi::RetLocal<v8::Value> getDisplay() {
        return display_.Get(v8::Isolate::GetCurrent());
    }

    //! TSDecl: @property @readonly contentAggregator: ContentAggregator
    ffi::RetLocal<v8::Value> getContentAggregator();

    //! TSDecl: @method close(): @promise(void)
    ffi::RetLocal<v8::Value> close();

    //! TSDecl: @method setTitle(title: string): @promise(void)
    ffi::RetLocal<v8::Value> setTitle(const std::string& title);

    //! TSDecl: @method resize(width: i32, height: i32): @promise(boolean)
    ffi::RetLocal<v8::Value> resize(int32_t width, int32_t height);

    //! TSDecl: @method requestBufferStateInfo(): @promise(string)
    ffi::RetLocal<v8::Value> requestBufferStateInfo();

    //! TSDecl: @method requestNextFrame(): @promise(u32)
    ffi::RetLocal<v8::Value> requestNextFrame();

    //! TSDecl: @method setMaxSize(width: i32, height: i32): @promise(void)
    ffi::RetLocal<v8::Value> setMaxSize(int32_t width, int32_t height);

    //! TSDecl: @method setMinSize(width: i32, height: i32): @promise(void)
    ffi::RetLocal<v8::Value> setMinSize(int32_t width, int32_t height);

    //! TSDecl: @method setMaximized(value: boolean): @promise(void)
    ffi::RetLocal<v8::Value> setMaximized(bool value);

    //! TSDecl: @method setMinimized(value: boolean): @promise(void)
    ffi::RetLocal<v8::Value> setMinimized(bool value);

    //! TSDecl: @method setFullscreen(value: boolean, monitor: Monitor): @promise(void)
    ffi::RetLocal<v8::Value> setFullscreen(bool value, const ffi::Opt<ffi::Class<Monitor>>& monitor);

    //! TSDecl: @method setAttachedCursor(cursor: Cursor): @promise(void)
    ffi::RetLocal<v8::Value> setAttachedCursor(const ffi::Class<Cursor>& cursor);

    //! TSDecl: @method getVideoDecodeCompatibleDevice(): @union(null, @import(multimedia) HWDeviceContext)
    ffi::RetLocal<v8::Value> getVideoDecodeCompatibleDevice();

private:
    SkISize                         dimensions_;
    v8::Global<v8::Object>          display_;
    std::shared_ptr<gl::Surface>    surface_;
    v8::Global<v8::Object>          content_aggregator_;
    v8::Global<v8::Value>           videodec_compatible_device_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_SURFACE_H
