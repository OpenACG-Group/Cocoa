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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_SCENE_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_SCENE_H

#include "Glamor/Layers/LayerTree.h"
#include "Utau/PixelFormatUtils.h"

#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/ffi/JSObject.h"
#include "Gallium/bindings/present/Types.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/Matrix.h"
#include "Gallium/bindings/renderer/SamplingOptions.h"
#include "Gallium/bindings/multimedia/Frame.h"

GALLIUM_BINDINGS_RENDERER_NS_BEGIN
class ImageFilter;
class ColorFilter;
class Path;
class Picture;
GALLIUM_BINDINGS_RENDERER_NS_END

GALLIUM_BINDINGS_PRESENT_NS_BEGIN

//! TSDecl: @class @nonconstructible Scene
class Scene : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Scene(std::unique_ptr<gl::LayerTree> tree) : layer_tree_(std::move(tree)) {}
    ~Scene() override = default;

    g_nodiscard std::unique_ptr<gl::LayerTree> TakeLayerTree();

    //! TSDecl: @method toString(): string
    ffi::RetLocal<v8::Value> toString();

private:
    std::unique_ptr<gl::LayerTree> layer_tree_;
};
//! TSDecl: @end

//! TSDecl: @enum VideoFrameViewResampler
enum class VideoFrameViewResampler
{
#define ITEM(k) k = static_cast<int>(utau::ScaleResampler::k)
    //! @tsdocbegin
    //! Nearest single sample. Fastest but low quality.
    //! @tsdocend
    //! TSDecl: @enumitem Nearest
    ITEM(kNearest),

    //! @tsdocbegin
    //! Bilinear interpolation. Slower but higher quality, best choice for most cases.
    //! @tsdocend
    //! TSDecl: @enumitem Bilinear
    ITEM(kBilinear),

    //! @tsdocbegin
    //! Bicubic interpolation. Slowest but high quality.
    //! @tsdocend
    //! TSDecl: @enumitem Bicubic
    ITEM(kBicubic)
#undef ITEM
};
//! TSDecl: @end

//! TSDecl: @class SceneBuilder
class SceneBuilder : public ffi::JSObject
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    //! TSDecl: @constructor(viewportCull: @import(renderer) Rect)
    explicit SceneBuilder(const renderer::RectAdapter& viewport_cull);
    ~SceneBuilder() override = default;

    //! TSDecl: @method build(): Scene
    ffi::RetLocal<v8::Value> build();

    //! TSDecl: @method pop(): SceneBuilder
    ffi::RetLocal<v8::Value> pop();

    //! TSDecl: @method pushOffset(x: f32, y: f32): SceneBuilder
    ffi::RetLocal<v8::Value> pushOffset(float x, float y);

    //! TSDecl: @method pushTransform(matrix: @import(renderer) Mat3x3): SceneBuilder
    ffi::RetLocal<v8::Value> pushTransform(const renderer::Mat3x3Adapter& matrix);

    //! TSDecl: @method addPicture(picture: @import(renderer) Picture, clipBounds: boolean): SceneBuilder
    ffi::RetLocal<v8::Value> addPicture(const ffi::Class<renderer::Picture>& picture, bool clip_bounds);

    //! TSDecl: @method pushOpacity(alpha: f32): SceneBuilder
    ffi::RetLocal<v8::Value> pushOpacity(float alpha);

    //! TSDecl: @method pushImageFilter(filter: @import(renderer) ImageFilter): SceneBuilder
    ffi::RetLocal<v8::Value> pushImageFilter(const ffi::Class<renderer::ImageFilter>& filter);

    //! TSDecl: @method pushBackdropFilter(filter: @import(renderer) ImageFilter,
    //! TSDecl:                            blendMode: @import(renderer) BlendMode,
    //! TSDecl:                            clipChildBounds: boolean): SceneBuilder
    ffi::RetLocal<v8::Value> pushBackdropFilter(const ffi::Class<renderer::ImageFilter>& filter,
                                                const ffi::Enum<SkBlendMode>& blendMode,
                                                bool clip_child_bounds);

    //! TSDecl: @method pushRectClip(shape: @import(renderer) Rect, antialias: boolean): SceneBuilder
    ffi::RetLocal<v8::Value> pushRectClip(const renderer::RectAdapter& shape, bool AA);

    //! TSDecl: @method pushRRectClip(shape: @import(renderer) RRect, antialias: boolean): SceneBuilder
    ffi::RetLocal<v8::Value> pushRRectClip(const renderer::RRectAdapter& shape, bool AA);

    //! TSDecl: @method pushPathClip(shape: @import(renderer) Path, op: @import(renderer) ClipOp,
    //! TSDecl:                      antialias: boolean): SceneBuilder
    ffi::RetLocal<v8::Value> pushPathClip(const ffi::Class<renderer::Path>& shape,
                                          const ffi::Enum<SkClipOp>& op, bool antialias);

    //! TSDecl: @method addVideoFrameView(frame: @import(multimedia) Frame,
    //! TSDecl:                           offset: @tuple(f32, f32), width: i32, height: i32,
    //! TSDecl:                           sampling: VideoFrameViewResampler): SceneBuilder
    ffi::RetLocal<v8::Value> addVideoFrameView(ffi::Class<multimedia::Frame> frame,
                                               std::tuple<float, float> offset,
                                               int32_t width, int32_t height,
                                               ffi::Enum<VideoFrameViewResampler> resampler);

    // TODO(present:sora): addGpuSurfaceView

private:
    void PushLayer(const std::shared_ptr<gl::ContainerLayer>& layer);
    void AddLayer(const std::shared_ptr<gl::Layer>& layer);

    template<typename LayerT, typename ...ArgsT>
    void PushLayer(ArgsT&&...args) {
        PushLayer(std::make_shared<LayerT>(std::forward<ArgsT>(args)...));
    }

    template<typename LayerT, typename ...ArgsT>
    void AddLayer(ArgsT&&...args) {
        AddLayer(std::make_shared<LayerT>(std::forward<ArgsT>(args)...));
    }

    std::unique_ptr<gl::LayerTree>                      layer_tree_;
    std::stack<std::shared_ptr<gl::ContainerLayer>>     layer_stack_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_SCENE_H
