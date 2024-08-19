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

#include "Gallium/bindings/multimedia/HWDeviceContext.h"
#include "Utau/HWDeviceContext.h"
GALLIUM_BINDINGS_MULTIMEDIA_NS_BEGIN

ffi::RetLocal<v8::Value> HWDeviceContext::MakeVulkan()
{
    AVBufferRef *context = utau::HWDeviceContext::MakeVulkan();
    if (!context)
        return ffi::Fail(ffi::kErr, "failed to create GPU device context");
    return ffi::JSObject::New<HWDeviceContext>(v8::Isolate::GetCurrent(), context);
}

ffi::Ret<void> HWDeviceContext::dispose()
{
    av_buffer_unref(&hwctx_);
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

GALLIUM_BINDINGS_MULTIMEDIA_NS_END
