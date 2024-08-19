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

#include "Core/Errors.h"
#include "Glamor/Layers/TransformLayer.h"
#include "Glamor/Layers/PictureLayer.h"
#include "Glamor/Layers/ImageFilterLayer.h"
#include "Glamor/Layers/BackdropFilterLayer.h"
#include "Glamor/Layers/RectClipLayer.h"
#include "Glamor/Layers/RRectClipLayer.h"
#include "Glamor/Layers/PathClipLayer.h"
#include "Glamor/Layers/OpacityLayer.h"
#include "Glamor/Layers/GpuSurfaceViewLayer.h"
#include "Glamor/Layers/ExternalTextureLayer.h"

#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/ColorFilter.h"
#include "Gallium/bindings/renderer/Path.h"
#include "Gallium/bindings/renderer/Picture.h"
#include "Gallium/bindings/present/Scene.h"

#include "Utau/FrameTextureConverter.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

std::unique_ptr<gl::LayerTree> Scene::TakeLayerTree()
{
    CHECK(layer_tree_);
    NotifyDisposeState(DisposeState::kDisposed);
    return std::move(layer_tree_);
}

ffi::RetLocal<v8::Value> Scene::toString()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return v8::String::NewFromUtf8(isolate, layer_tree_->ToString().c_str()).ToLocalChecked();
}

SceneBuilder::SceneBuilder(const renderer::RectAdapter& viewport_cull)
    : layer_tree_(std::make_unique<gl::LayerTree>(*viewport_cull))
{
}

void SceneBuilder::PushLayer(const std::shared_ptr<gl::ContainerLayer>& layer)
{
    CHECK(layer);
    if (!layer_stack_.empty())
        layer_stack_.top()->AppendChildLayer(layer);
    layer_stack_.push(layer);
    if (!layer_tree_->GetRootLayer())
        layer_tree_->SetRootLayer(layer);
}

void SceneBuilder::AddLayer(const std::shared_ptr<gl::Layer>& layer)
{
    if (layer_stack_.empty())
    {
        // Since the root node must be a container layer, we add a do-nothing container layer
        // if the user does not provide one.
        PushLayer<gl::TransformLayer>(SkMatrix::Translate(0, 0));
    }

    layer_stack_.top()->AppendChildLayer(layer);
}

ffi::RetLocal<v8::Value> SceneBuilder::build()
{
    NotifyDisposeState(DisposeState::kDisposed);
    while (!layer_stack_.empty())
        layer_stack_.pop();
    return ffi::JSObject::New<Scene>(v8::Isolate::GetCurrent(), std::move(layer_tree_));
}

ffi::RetLocal<v8::Value> SceneBuilder::pop()
{
    if (layer_stack_.empty())
        return ffi::Fail(ffi::kErr, "invalid pop operation: layer stack is empty");
    layer_stack_.pop();
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> SceneBuilder::pushOffset(float x, float y)
{
    PushLayer<gl::TransformLayer>(SkMatrix::Translate(x, y));
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> SceneBuilder::pushTransform(const renderer::Mat3x3Adapter& matrix)
{
    PushLayer<gl::TransformLayer>(*matrix);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> SceneBuilder::pushOpacity(float alpha)
{
    PushLayer<gl::OpacityLayer>(std::clamp(alpha, 0.0f, 1.0f));
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
SceneBuilder::pushImageFilter(const ffi::Class<renderer::ImageFilter>& filter)
{
    PushLayer<gl::ImageFilterLayer>(filter->GetSkImageFilter());
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
SceneBuilder::pushBackdropFilter(const ffi::Class<renderer::ImageFilter>& filter,
                                 const ffi::Enum<SkBlendMode>& blendMode,
                                 bool clip_child_bounds)
{
    PushLayer<gl::BackdropFilterLayer>(filter->GetSkImageFilter(), *blendMode, clip_child_bounds);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> SceneBuilder::pushRectClip(const renderer::RectAdapter& shape, bool AA)
{
    PushLayer<gl::RectClipLayer>(*shape, AA);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value> SceneBuilder::pushRRectClip(const renderer::RRectAdapter& shape, bool AA)
{
    PushLayer<gl::RRectClipLayer>(*shape, AA);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
SceneBuilder::pushPathClip(const ffi::Class<renderer::Path>& shape,
                           const ffi::Enum<SkClipOp>& op, bool antialias)
{
    PushLayer<gl::PathClipLayer>(shape->GetSkPath(), *op, antialias);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
SceneBuilder::addPicture(const ffi::Class<renderer::Picture>& picture, bool clip_bounds)
{
    AddLayer<gl::PictureLayer>(clip_bounds, picture->GetSkPicture());
    return GetThisHandle(v8::Isolate::GetCurrent());
}

ffi::RetLocal<v8::Value>
SceneBuilder::addVideoFrameView(ffi::Class<multimedia::Frame> frame,
                                std::tuple<float, float> offset, int32_t width, int32_t height,
                                ffi::Enum<VideoFrameViewResampler> resampler)
{
    if (frame->GetDisposeState() != DisposeState::kNot)
        return ffi::Fail(ffi::kErr, "provided `multimedia.Frame` instance has been disposed");

    auto [offset_x, offset_y] = offset;
    std::shared_ptr external_layer = utau::WrapFrameToGLExternalLayer(
        frame->GetAVFrame(),
        SkPoint::Make(offset_x, offset_y),
        SkISize::Make(width, height),
        static_cast<utau::ScaleResampler>(resampler.GetInteger())
    );
    if (!external_layer)
        return ffi::Fail(ffi::kErr, "failed to create an external layer for the video frame");

    AddLayer(external_layer);
    return GetThisHandle(v8::Isolate::GetCurrent());
}

GALLIUM_BINDINGS_PRESENT_NS_END
