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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_MONITOR_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_MONITOR_H

#include "Glamor/Monitor.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/bindings/present/Types.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

//! TSDecl: @interface MonitorPropertySet
//! TSDecl: @property logicalX: i32
//! TSDecl: @property logicalY: i32
//! TSDecl: @property physicalWidth: i32
//! TSDecl: @property physicalHeight: i32
//! TSDecl: @property subpixel: MonitorSubpixel
//! TSDecl: @property manufactureName: string
//! TSDecl: @property modelName: string
//! TSDecl: @property transform: MonitorTransform
//! TSDecl: @property modeFlags: u32
//! TSDecl: @property modeWidth: i32
//! TSDecl: @property modeHeight: i32
//! TSDecl: @property refreshRate: i32
//! TSDecl: @property scaleFactor: i32
//! TSDecl: @property connectorName: string
//! TSDecl: @property description: string
//! TSDecl: @end

//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) Monitor
class Monitor : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Monitor(std::shared_ptr<gl::Monitor> monitor);
    ~Monitor() override = default;

    g_nodiscard std::shared_ptr<gl::Monitor> GetGLMonitor() const {
        return monitor_;
    }

    //! TSDecl: @method requestPropertySet(): @promise(void)
    ffi::RetLocal<v8::Value> requestPropertySet();

private:
    std::shared_ptr<gl::Monitor> monitor_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_MONITOR_H
