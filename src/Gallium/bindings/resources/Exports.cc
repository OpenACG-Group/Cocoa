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

#include <unordered_map>

#include "Gallium/bindings/resources/Exports.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDINGS_RESOURCES_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
#define V(x) static_cast<uint32_t>(x)

    std::unordered_map<std::string_view, uint32_t> constants = {
        { "CRPKG_SOURCE_TYPE_UINT8ARRAY", V(CRPKGSourceType::kUint8Array) },
        { "CRPKG_SOURCE_TYPE_FILEPATH", V(CRPKGSourceType::kFilePath) },
        { "CRPKG_SOURCE_TYPE_CRPKG_STORAGE", V(CRPKGSourceType::kCRPKGStorage) }
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> constants_object = binder::to_v8(isolate, constants);

    instance->Set(ctx, v8::String::NewFromUtf8Literal(isolate, "Constants"),
                  constants_object).Check();
}

GALLIUM_BINDINGS_RESOURCES_NS_END
