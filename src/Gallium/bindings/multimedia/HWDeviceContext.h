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

#ifndef COCOA_GALLIUM_BINDINGS_MULTIMEDIA_HWDEVICECONTEXT_H
#define COCOA_GALLIUM_BINDINGS_MULTIMEDIA_HWDEVICECONTEXT_H

#include "Gallium/bindings/multimedia/Types.h"
#include "Gallium/ffi/JSObject.h"

GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

//! @tsdocbegin
//! Represents a hardware device for video decoding or encoding. Instances of `HWDeviceContext`
//! are references to the underlying device handle, and the underlying device resources will be
//! disposed when there are no references to it.
//!
//! After the `HWDeviceContext` is passed to consumers, like `CodecContext` or `HWFramesContext`,
//! they will create references to the underlying device handle internally, and it is safe to
//! dispose the `HWDeviceContext` instance itself.
//!
//! To create a new `HWDeviceContext`:
//!   - call `HWDeviceContext.MakeVulkan()` to create a new context. This method creates a new
//!     separated Vulkan logical device.
//!   - obtain an instance from `present.Surface.getVideoDecodeCompatibleDevice()`. This method
//!     returns a deocde-only context that shares the same logical device with the corresponding
//!     `present.Surface` surface. It makes the zerocopy texture sharing available on that surface.
//! @tsdocend
//! TSDecl: @class @nonconstructible HWDeviceContext
class HWDeviceContext : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! @tsdocbegin
    //! Creates a new context using a separated Vulkan logical device.
    //! Throws an exception on failure.
    //! @tsdocend
    //! TSDecl: @method @static MakeVulkan(): HWDeviceContext
    static ffi::RetLocal<v8::Value> MakeVulkan();

    explicit HWDeviceContext(AVBufferRef *hwctx) : hwctx_(hwctx) {}
    ~HWDeviceContext() override = default;

    g_nodiscard AVBufferRef *GetContext() const {
        return hwctx_;
    }

    //! @tsdocbegin
    //! Dispose this reference to the underlying device handle.
    //! @tsdocend
    //! TSDecl: @method dispose(): void
    ffi::Ret<void> dispose();

private:
    AVBufferRef *hwctx_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
#endif //COCOA_GALLIUM_BINDINGS_MULTIMEDIA_HWDEVICECONTEXT_H
