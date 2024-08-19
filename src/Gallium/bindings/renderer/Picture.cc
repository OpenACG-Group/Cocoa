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

#include "Gallium/bindings/renderer/Canvas.h"
#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/renderer/Shader.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Value> Picture::MakePlaceholder(const RectAdapter& cull)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Picture>(isolate, SkPicture::MakePlaceholder(*cull));
}

ffi::Ret<void> Picture::playback(const ffi::Class<Canvas>& canvas)
{
    SkCanvas *sk_canvas = (*canvas)->GetSkCanvas();
    if (!sk_canvas)
        return ffi::Fail(ffi::kErr, "provided canvas has been disposed");
    picture_->playback(sk_canvas);
    return {};
}

ffi::RetLocal<v8::Value> Picture::getCullRect()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!cache_cull_rect_.IsEmpty())
        return cache_cull_rect_.Get(isolate);

    auto cull = CreateJSRect(isolate, picture_->cullRect());
    CHECK(!cull.IsEmpty() && cull->IsObject());
    cache_cull_rect_.Reset(isolate, cull.As<v8::Object>());
    return cull;
}

ffi::RetLocal<v8::Value> Picture::makeShader(const ffi::Enum<SkTileMode>& tmx,
                                             const ffi::Enum<SkTileMode>& tmy,
                                             const ffi::Enum<SkFilterMode>& mode,
                                             const ffi::Opt<Mat3x3Adapter>& local_matrix,
                                             const ffi::Opt<RectAdapter>& tile_rect)
{
    sk_sp<SkShader> shader = picture_->makeShader(*tmx, *tmy, *mode,
                                                 local_matrix ? &(**local_matrix) : nullptr,
                                                 tile_rect ? &(**tile_rect) : nullptr);
    if (!shader)
        return ffi::Fail(ffi::kErr, "failed to make shader from Picture: invalid arguments");
    return ffi::JSObject::New<Shader>(v8::Isolate::GetCurrent(), shader);
}

sk_sp<SkData> Picture::OnSerializeImpl(SkSerialProcs *procs)
{
    return picture_->serialize(procs);
}

GALLIUM_BINDINGS_RENDERER_NS_END
