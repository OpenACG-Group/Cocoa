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

#include "Gallium/bindings/stream/Module.h"
GALLIUM_BINDINGS_NS_BEGIN

StreamModule::StreamModule()
    : ffi::NativeModule("stream", "Basic stream API")
{
}

ffi::LocalExports StreamModule::OnBuildExports(v8::Isolate *isolate)
{
    ffi::LocalExports exports;

    InsertScriptExports(isolate, "internal://natives/stream.js", exports);

    return exports;
}

GALLIUM_BINDINGS_NS_END
