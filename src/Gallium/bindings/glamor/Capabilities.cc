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

#include "Gallium/bindings/glamor/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

v8::Local<v8::Value> QueryCapabilities(uint32_t cap)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (cap > static_cast<uint32_t>(Capabilities::kLast))
        g_throw(RangeError, "Invalid enumeration value for argument `cap`");

#define V(x) binder::to_v8(isolate, x)
    auto cap_enum = static_cast<Capabilities>(cap);

    gl::GlobalScope& context = gl::GlobalScope::Ref();
    gl::ContextOptions& options = context.GetOptions();
    switch (cap_enum)
    {
    case Capabilities::kHWComposeEnabled:
        return V(!options.GetDisableHWCompose());
    case Capabilities::kProfilerEnabled:
        return V(options.GetEnableProfiler());
    case Capabilities::kProfilerMaxSamples:
        return V(options.GetProfilerRingBufferThreshold());
    case Capabilities::kMessageQueueProfilingEnabled:
        return V(options.GetProfileRenderHostTransfer());
    }

    MARK_UNREACHABLE("Invalid enumeration value");
}

GALLIUM_BINDINGS_GLAMOR_NS_END
