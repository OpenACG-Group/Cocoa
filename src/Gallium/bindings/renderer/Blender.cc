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

#include "include/core/SkData.h"
#include "include/effects/SkBlenders.h"

#include "Gallium/bindings/renderer/Blender.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> Blender::Mode(const ffi::Enum<SkBlendMode>& mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Blender>(isolate, SkBlender::Mode(*mode));
}

ffi::RetLocal<v8::Value> Blender::Arithmetic(float k1, float k2, float k3, float k4, bool enforce_pmcolor)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Blender>(isolate, SkBlenders::Arithmetic(k1, k2, k3, k4, enforce_pmcolor));
}

sk_sp<SkData> Blender::OnSerializeImpl(SkSerialProcs *procs)
{
    return blender_->serialize(procs);
}

GALLIUM_BINDINGS_RENDERER_NS_END
