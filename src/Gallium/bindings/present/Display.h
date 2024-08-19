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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_DISPLAY_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_DISPLAY_H

#include "Glamor/Display.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Interface.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/bindings/present/Types.h"
#include "Gallium/bindings/renderer/Pixmap.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

//! TSDecl: @interface SurfaceCreationOptions
struct SurfaceCreationOptions
{
    //! TSDecl: @property @optional enableGpuPipeline: boolean
    ffi::Opt<bool> enable_gpu_pipeline;

    //! TSDecl: @property @optional enableGpuVideoDecodeCompatible: boolean
    ffi::Opt<bool> enable_gpu_video_decode_compatible;
};
//! TSDecl: @end

//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) Display
class Display : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Display(std::shared_ptr<gl::Display> display);
    ~Display() override = default;

    //! TSDecl: @method close(): @promise(void)
    ffi::RetLocal<v8::Value> close();

    //! TSDecl: @method requestMonitorList(): @promise(@array(Monitor))
    ffi::RetLocal<v8::Value> requestMonitorList();

    //! TSDecl: @property @readonly defaultCursorTheme: CursorTheme
    ffi::RetLocal<v8::Value> getDefaultCursorTheme();

    //! TSDecl: @method loadCursorTheme(name: string, size: i32): @promise(CursorTheme)
    ffi::RetLocal<v8::Value> loadCursorTheme(const std::string& name, int32_t size);

    //! TSDecl: @method createCursor(pixmap: @import(renderer) Pixmap, hotspotX: i32, hotspotY: i32): @promise(Cursor)
    ffi::RetLocal<v8::Value> createCursor(renderer::PixmapAdapter pixmap,
                                          int32_t hotspot_x, int32_t hotspot_y);

    //! TSDecl: @method createSurface(width: i32, height: i32, options: SurfaceCreationOptions): @promise(Surface)
    ffi::RetLocal<v8::Value> createSurface(int32_t width, int32_t height,
                                           ffi::IFace<SurfaceCreationOptions> options);

private:
    using MonitorMap = std::map<std::shared_ptr<gl::Monitor>, v8::Global<v8::Object>>;

    std::shared_ptr<gl::Display>    display_;
    MonitorMap                      monitor_map_;
    v8::Global<v8::Value>           default_cursor_theme_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_DISPLAY_H
