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

#include "Gallium/ffi/Module.h"
#include "Gallium/bindings/workers/Module.h"
#include "Gallium/bindings/workers/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

WorkersModule::WorkersModule()
    : ffi::NativeModule("workers", "Multithreading support")
{
}

ffi::LocalExports WorkersModule::OnBuildExports(v8::Isolate *isolate)
{
    using namespace workers_wrap;

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    auto worker = ffi::DefineClass<WorkerWrap>(isolate)
        .MethodStatic("MakeFromURL", WorkerWrap::MakeFromURL)
        .Property<v8::Local<v8::Value>>("port", &WorkerWrap::getPort, nullptr)
        .Finalize()
        ->GetFunction(ctx).ToLocalChecked();

    auto message_port = ffi::DefineClass<MessagePortWrap>(isolate)
        .Inherit<EventEmitterBase>()
        .MethodStatic("MakeConnectedPair", MessagePortWrap::MakeConnectedPair)
        .Method("close", &MessagePortWrap::close)
        .Method("postMessage", &MessagePortWrap::postMessage)
        .Finalize()
        ->GetFunction(ctx).ToLocalChecked();

    return ffi::LocalExports {
        { "Worker", worker },
        { "MessagePort", message_port }
    };
}

GALLIUM_BINDINGS_NS_END
