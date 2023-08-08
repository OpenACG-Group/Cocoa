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

#include "Gallium/bindings/utau/Exports.h"
GALLIUM_BINDINGS_UTAU_NS_BEGIN

HWFramesContextRef::HWFramesContextRef(AVBufferRef *ref)
    : ref_(av_buffer_ref(ref))
{
}

HWFramesContextRef::~HWFramesContextRef()
{
    dispose();
}

void HWFramesContextRef::dispose()
{
    if (ref_)
        av_buffer_unref(&ref_);
}

v8::Local<v8::Value> HWFramesContextRef::clone()
{
    if (!ref_)
        g_throw(Error, "Reference has been disposed");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<HWFramesContextRef>(isolate, ref_);
}

GALLIUM_BINDINGS_UTAU_NS_END
