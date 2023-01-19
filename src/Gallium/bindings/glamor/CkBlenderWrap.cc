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

#include "include/core/SkBlender.h"
#include "include/effects/SkBlenders.h"

#include "Gallium/binder/Class.h"
#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> CkBlenderWrap::Mode(int32_t mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (mode < 0 || mode > static_cast<int32_t>(SkBlendMode::kLastMode))
        g_throw(RangeError, "Invalid enumeration value for argument `mode`");

    return binder::Class<CkBlenderWrap>::create_object(
            isolate, SkBlender::Mode(static_cast<SkBlendMode>(mode)));
}

v8::Local<v8::Value> CkBlenderWrap::Arithmetic(float k1, float k2, float k3, float k4, bool enforcePM)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::Class<CkBlenderWrap>::create_object(
            isolate, SkBlenders::Arithmetic(k1, k2, k3, k4, enforcePM));
}

GALLIUM_BINDINGS_GLAMOR_NS_END
