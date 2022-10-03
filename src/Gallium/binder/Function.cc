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

#include "Gallium/binder/Function.h"
#include "Gallium/Runtime.h"
GALLIUM_BINDER_NS_BEGIN
namespace detail {

void external_data::destroy_all(v8::Isolate *isolate)
{
    Runtime *runtime = Runtime::GetBareFromIsolate(isolate);
    CHECK(runtime);
    runtime->DeleteExternalValueHolders();
}

void external_data::register_external(value_holder_base *value)
{
    CHECK(value && value->isolate);

    Runtime *runtime = Runtime::GetBareFromIsolate(value->isolate);
    CHECK(runtime);
    runtime->RegisterExternalValueHolder(value);
}

void external_data::unregister_external(value_holder_base *value)
{
    CHECK(value && value->isolate);

    Runtime *runtime = Runtime::GetBareFromIsolate(value->isolate);
    CHECK(runtime);
    runtime->UnregisterExternalValueHolder(value);
}

} // namespace detail
GALLIUM_BINDER_NS_END
