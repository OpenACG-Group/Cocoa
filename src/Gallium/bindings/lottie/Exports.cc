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

#include <string_view>
#include <unordered_map>

#include "Gallium/bindings/lottie/Exports.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDINGS_LOTTIE_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
    std::unordered_map<std::string_view, uint32_t> constants = {
        { "ANIMATION_BUILDER_FLAGS_DEFER_IMAGE_LOADING", skottie::Animation::Builder::kDeferImageLoading },
        { "ANIMATION_BUILDER_FLAGS_PREFER_EMBEDDED_FONTS", skottie::Animation::Builder::kPreferEmbeddedFonts },
        { "ANIMATION_RENDER_FLAG_SKIP_TOP_LEVEL_ISOLATION", skottie::Animation::kSkipTopLevelIsolation },
        { "ANIMATION_RENDER_FLAG_DISABLE_TOP_LEVEL_CLIPPING", skottie::Animation::kDisableTopLevelClipping },
        { "LOGGER_LEVEL_WARNING", static_cast<uint32_t>(skottie::Logger::Level::kWarning) },
        { "LOGGER_LEVEL_ERROR", static_cast<uint32_t>(skottie::Logger::Level::kError) }
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> constants_object = binder::to_v8(isolate, constants);

    instance->Set(ctx, v8::String::NewFromUtf8Literal(isolate, "Constants"),
                  constants_object).Check();
}

GALLIUM_BINDINGS_LOTTIE_NS_END
