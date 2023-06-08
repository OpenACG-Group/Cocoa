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

#include "Gallium/bindings/lottie/Exports.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
#include "Gallium/binder/ThrowExcept.h"
GALLIUM_BINDINGS_LOTTIE_NS_BEGIN

void AnimationWrap::seekFrame(double t)
{
    animation_->seekFrame(t);
}

void AnimationWrap::seekFrameTime(double t)
{
    animation_->seekFrameTime(t);
}

void AnimationWrap::render(v8::Local<v8::Value> canvas, v8::Local<v8::Value> dst,
                           uint32_t flags)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    glamor_wrap::CkCanvas *canvas_wrap =
            binder::UnwrapObject<glamor_wrap::CkCanvas>(isolate, canvas);
    if (!canvas_wrap)
        g_throw(TypeError, "Argument `canvas` must be an instance of `CkCanvas`");

    std::optional<SkRect> dst_rect;
    if (!dst->IsNullOrUndefined())
        dst_rect = glamor_wrap::ExtractCkRect(isolate, dst);

    animation_->render(canvas_wrap->GetCanvas(),
                       dst_rect ? &(*dst_rect) : nullptr,
                       flags);
}

GALLIUM_BINDINGS_LOTTIE_NS_END
