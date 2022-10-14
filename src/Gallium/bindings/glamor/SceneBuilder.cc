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

#include "Glamor/Layers/TransformLayer.h"
#include "Glamor/Layers/PictureLayer.h"
#include "Glamor/Layers/TextureLayer.h"
#include "Glamor/Layers/ImageFilterLayer.h"
#include "Glamor/Layers/BackdropFilterLayer.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

SceneBuilder::SceneBuilder(int32_t width, int32_t height)
    : width_(width)
    , height_(height)
{
}

v8::Local<v8::Object> SceneBuilder::getSelfHandle()
{
    if (self_handle_.IsEmpty())
    {
        v8::Isolate *isolate = v8::Isolate::GetCurrent();
        self_handle_.Reset(isolate, binder::Class<SceneBuilder>::find_object(isolate, this));
        CHECK(!self_handle_.IsEmpty());

        // Make this weak-reference to avoid circular reference.
        self_handle_.SetWeak();
    }
    return self_handle_.Get(v8::Isolate::GetCurrent());
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

    v8::Local<v8::Value> object = binder::Class<Scene>::create_object(isolate, layer_tree_,
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
    return getSelfHandle();
}

v8::Local<v8::Value> SceneBuilder::pushOffset(SkScalar x, SkScalar y)
{
    pushLayer(std::make_shared<gl::TransformLayer>(SkMatrix::Translate(x, y)));
    return getSelfHandle();
}

v8::Local<v8::Value> SceneBuilder::pushImageFilter(v8::Local<v8::Value> filter)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkImageFilterWrap>::unwrap_object(isolate, filter);
    if (!wrapper)
        g_throw(TypeError, "Argument 'filter' must be an instance of `CkImageFilter`");

    CHECK(wrapper->getSkiaObject());
    pushLayer(std::make_shared<gl::ImageFilterLayer>(wrapper->getSkiaObject()));

    return getSelfHandle();
}

v8::Local<v8::Value> SceneBuilder::pushBackdropFilter(v8::Local<v8::Value> filter,
                                                      int32_t blendMode,
                                                      bool autoChildClip)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrapper = binder::Class<CkImageFilterWrap>::unwrap_object(isolate, filter);
    if (!wrapper)
        g_throw(TypeError, "Argument 'filter' must be an instance of `CkImageFilter`");

    CHECK(wrapper->getSkiaObject());

    if (blendMode < 0 || blendMode > static_cast<int>(SkBlendMode::kLastMode))
        g_throw(RangeError, "Argument 'blendMode' has an invalid enumeration value");

    auto mode = static_cast<SkBlendMode>(blendMode);
    pushLayer(std::make_shared<gl::BackdropFilterLayer>(wrapper->getSkiaObject(),
                                                        mode, autoChildClip));

    return getSelfHandle();
}

v8::Local<v8::Value> SceneBuilder::addPicture(v8::Local<v8::Value> picture,
                                              bool autoFastClip,
                                              SkScalar dx, SkScalar dy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkPictureWrap *unwrapped = binder::Class<CkPictureWrap>::unwrap_object(isolate, picture);
    if (unwrapped == nullptr)
        g_throw(TypeError, "\'picture\' must be an instance of CkPicture");

    addLayer(std::make_shared<gl::PictureLayer>(SkPoint::Make(dx, dy),
                                                autoFastClip,
                                                unwrapped->getPicture()));
    return getSelfHandle();
}

#define EV(v)   static_cast<typename std::underlying_type<Sampling>::type>(v)

v8::Local<v8::Value> SceneBuilder::addTexture(int64_t textureId,
                                              SkScalar dx,
                                              SkScalar dy,
                                              SkScalar width,
                                              SkScalar height,
                                              int32_t sampling)
{
    SkPoint offset = SkPoint::Make(dx, dy);
    SkISize size = SkSize::Make(width, height).toRound();
    SkSamplingOptions sampling_options = SamplingToSamplingOptions(sampling);

    addLayer(std::make_shared<gl::TextureLayer>(textureId, offset, size, sampling_options));
    return getSelfHandle();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
