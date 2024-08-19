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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_PRESENTTHREAD_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_PRESENTTHREAD_H

#include "Glamor/PresentThread.h"

#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/present/Types.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

//! TSDecl: @class @nonconstructible PresentThread
class PresentThread : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit PresentThread(gl::PresentThread *thread) : thread_(thread) {}
    ~PresentThread() override = default;

    //! TSDecl: @method @static Start(): PresentThread
    static ffi::RetLocal<v8::Value> Start();

    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

    //! TSDecl: @method collect(): void
    ffi::Ret<void> collect();

    //! TSDecl: @method traceResourcesJSON(): @promise(string)
    ffi::RetLocal<v8::Value> traceResourcesJSON();

    //! TSDecl: @method createDisplay(): @promise(Display)
    ffi::RetLocal<v8::Value> createDisplay();

    // TODO(sora): complete this.

private:
    gl::PresentThread *thread_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_PRESENTTHREAD_H
