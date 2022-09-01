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

#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_NS_BEGIN

void CoreSetInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *i = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = i->GetCurrentContext();

    v8::Local<v8::Array> argv = v8::Array::New(i);
    instance->Set(ctx, binder::to_v8(i, "args"), argv).Check();
    auto pass = prop::Get()->next("Runtime")->next("Script")
                ->next("Pass")->as<PropertyArrayNode>();
    int32_t index = 0;
    for (const auto& node : *pass)
    {
        auto str = node->as<PropertyDataNode>()->extract<std::string>();
        argv->Set(ctx, index++, binder::to_v8(i, str)).Check();
    }

    v8::Local<v8::Object> property = PropertyWrap::GetWrap(i, prop::Get());
    instance->Set(ctx, binder::to_v8(i, "property"), property).Check();
}

GALLIUM_BINDINGS_NS_END
