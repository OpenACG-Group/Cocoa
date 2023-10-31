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

#include "Gallium/bindings/glamor/Scene.h"
#include "Gallium/bindings/glamor/SceneBuilder.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/CkPathWrap.h"
#include "Gallium/bindings/glamor/CkSurfaceContentTracker.h"
#include "Gallium/bindings/utau/Exports.h"

#include "Glamor/Layers/TransformLayer.h"
#include "Glamor/Layers/PictureLayer.h"
#include "Glamor/Layers/ImageFilterLayer.h"
#include "Glamor/Layers/BackdropFilterLayer.h"
#include "Glamor/Layers/RectClipLayer.h"
#include "Glamor/Layers/RRectClipLayer.h"
#include "Glamor/Layers/PathClipLayer.h"
#include "Glamor/Layers/OpacityLayer.h"
#include "Glamor/Layers/GpuSurfaceViewLayer.h"

#include "Utau/VideoFrameGLEmbedder.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

SceneBuilder::SceneBuilder(int32_t width, int32_t height)
    : width_(width)
    , height_(height)
{
}

void SceneBuilder::pushLayer(const std::shared_ptr<gl::ContainerLayer>& layer)
{
    CHECK(layer && "Invalid layer");

    if (!layer_stack_.empty())
        layer_stack_.top()->AppendChildLayer(layer);

    layer_stack_.push(layer);

    // The first layer pushed into stack will be the root layer.
    if (layer_tree_ == nullptr)
        layer_tree_ = layer;
}

void SceneBuilder::addLayer(const std::shared_ptr<gl::Layer>& layer)
{
    CHECK(layer && "Invalid layer");
    if (layer_stack_.empty())
        g_throw(Error, "Inserting a container layer before adding other layers is required");
    layer_stack_.top()->AppendChildLayer(layer);
}

v8::Local<v8::Value> SceneBuilder::build()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!layer_tree_)
        g_throw(Error, "Building an empty scene");

    v8::Local<v8::Value> object = binder::NewObject<Scene>(isolate, layer_tree_,
                                                                      SkISize::Make(width_, height_));
    // Since the `Scene` object is created, `SceneBuilder` is not available anymore
    layer_tree_.reset();
    while (!layer_stack_.empty())
        layer_stack_.pop();

    return object;
}

v8::Local<v8::Value> SceneBuilder::pop()
{
    if (layer_stack_.empty())
        g_throw(Error, "Empty layer stack");
    layer_stack_.pop();
    return GetObjectWeakReference().Get(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> SceneBuilder::pushOffset(SkScalar x, SkScalar y)
{
    pushLayer(std::make_shared<gl::TransformLayer>(SkMatrix::Translate(x, y)));
    return GetObjectWeakReference().Get(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> SceneBuilder::pushRotate(SkScalar rad, SkScalar pivotX, SkScalar pivotY)
{
    pushLayer(std::make_shared<gl::TransformLayer>(
            SkMatrix::RotateDeg(SkRadiansToDegrees(rad), SkPoint::Make(pivotX, pivotY))));
    return GetObjectWeakReference().Get(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> SceneBuilder::pushTransform(v8::Local<v8::Value> matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::UnwrapObject<CkMatrix>(isolate, matrix);
    if (!wrapper)
        g_throw(TypeError, "Argument `matrix` must be an instance of `CkMatrix`");
    pushLayer(std::make_shared<gl::TransformLayer>(wrapper->GetMatrix()));
    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::pushOpacity(SkScalar alpha)
{
    if (alpha < 0 || alpha > 1)
        g_throw(RangeError, "Invalid alpha value, must between [0, 1]");

    pushLayer(std::make_shared<gl::OpacityLayer>(alpha * 255));
    return GetObjectWeakReference().Get(v8::Isolate::GetCurrent());
}

v8::Local<v8::Value> SceneBuilder::pushImageFilter(v8::Local<v8::Value> filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::UnwrapObject<CkImageFilterWrap>(isolate, filter);
    if (!wrapper)
        g_throw(TypeError, "Argument 'filter' must be an instance of `CkImageFilter`");

    CHECK(wrapper->GetSkObject());
    pushLayer(std::make_shared<gl::ImageFilterLayer>(wrapper->GetSkObject()));

    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::pushBackdropFilter(v8::Local<v8::Value> filter,
                                                      int32_t blendMode,
                                                      bool autoChildClip)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::UnwrapObject<CkImageFilterWrap>(isolate, filter);
    if (!wrapper)
        g_throw(TypeError, "Argument 'filter' must be an instance of `CkImageFilter`");

    CHECK(wrapper->GetSkObject());

    if (blendMode < 0 || blendMode > static_cast<int>(SkBlendMode::kLastMode))
        g_throw(RangeError, "Argument 'blendMode' has an invalid enumeration value");

    auto mode = static_cast<SkBlendMode>(blendMode);
    pushLayer(std::make_shared<gl::BackdropFilterLayer>(wrapper->GetSkObject(),
                                                        mode, autoChildClip));

    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::pushRectClip(v8::Local<v8::Value> shape,
                                                bool AA)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkRect rect_shape = ExtractCkRect(isolate, shape);

    pushLayer(std::make_shared<gl::RectClipLayer>(rect_shape, AA));
    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::pushRRectClip(v8::Local<v8::Value> shape,
                                                 bool AA)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkRRect rrect_shape = ExtractCkRRect(isolate, shape);

    pushLayer(std::make_shared<gl::RRectClipLayer>(rrect_shape, AA));
    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::pushPathClip(v8::Local<v8::Value> shape,
                                                int32_t op,
                                                bool antialias)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrap = binder::UnwrapObject<CkPath>(isolate, shape);
    if (!wrap)
        g_throw(TypeError, "Argument `shape` must be a CkPath");

    if (op < 0 || op > static_cast<int>(SkClipOp::kMax_EnumValue))
        g_throw(RangeError, "Invalid enumeration value for argument `op`");

    pushLayer(std::make_shared<gl::PathClipLayer>(wrap->GetPath(),
                                                  static_cast<SkClipOp>(op),
                                                  antialias));

    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::addPicture(v8::Local<v8::Value> picture, bool autoFastClip)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *unwrapped = binder::UnwrapObject<CkPictureWrap>(isolate, picture);
    if (unwrapped == nullptr)
        g_throw(TypeError, "Argument `picture` must be a CkPicture");

    addLayer(std::make_shared<gl::PictureLayer>(autoFastClip, unwrapped->getPicture()));
    return GetObjectWeakReference().Get(isolate);
}

#define EV(v)   static_cast<typename std::underlying_type<Sampling>::type>(v)

v8::Local<v8::Value> SceneBuilder::addVideoBuffer(v8::Local<v8::Value> vbo,
                                                  SkScalar dx,
                                                  SkScalar dy,
                                                  SkScalar width,
                                                  SkScalar height,
                                                  int32_t sampling)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::UnwrapObject<utau_wrap::VideoBufferWrap>(isolate, vbo);
    if (!wrapper)
        g_throw(TypeError, "Argument `vbo` must be an instance of utau.VideoBuffer");

    utau::VideoFrameGLEmbedder *embedder = utau::GlobalContext::Ref().GetVideoFrameGLEmbedder();
    CHECK(embedder);

    std::shared_ptr<gl::ExternalTextureLayer> layer =
            embedder->Commit(wrapper->GetBuffer(),
                             SkPoint::Make(dx, dy),
                             SkSize::Make(width, height).toRound(),
                             SamplingToSamplingOptions(sampling));
    if (!layer)
        g_throw(Error, "Failed to generate an external texture layer for video");

    addLayer(layer);

    return GetObjectWeakReference().Get(isolate);
}

v8::Local<v8::Value> SceneBuilder::addGpuSurfaceView(v8::Local<v8::Value> surface_id,
                                                     v8::Local<v8::Value> dst_rect,
                                                     v8::Local<v8::Value> wait_sem_id,
                                                     v8::Local<v8::Value> signal_sem_id,
                                                     v8::Local<v8::Value> content_tracker)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    gl::GpuSurfaceViewLayer::ContentTracker *ptracker = nullptr;
    if (!content_tracker->IsNullOrUndefined())
    {
        auto *wrap = binder::UnwrapObject<CkSurfaceContentTracker>(isolate, content_tracker);
        if (!wrap)
        {
            g_throw(TypeError, "Argument `contentTracker` must be null or"
                               " an instance of CkSurfaceContentTracker");
        }
        ptracker = wrap->GetTracker();
        CHECK(ptracker);
    }

    auto from_bigint = [](v8::Local<v8::Value> v) {
        if (!v->IsBigInt())
            g_throw(TypeError, "Resource ID must be a bigint value");
        bool lossless;
        int64_t integral = v.As<v8::BigInt>()->Int64Value(&lossless);
        if (!lossless)
            g_throw(RangeError, "Resource ID is out of range");
        return integral;
    };

    SkRect sk_dst_rect = ExtractCkRect(isolate, dst_rect);
    addLayer(std::make_shared<gl::GpuSurfaceViewLayer>(
            from_bigint(surface_id),
            sk_dst_rect,
            from_bigint(wait_sem_id),
            from_bigint(signal_sem_id),
            ptracker
    ));
    return GetObjectWeakReference().Get(isolate);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
