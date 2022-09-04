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

#ifndef COCOA_GALLIUM_BINDINGS_GLAMOR_SCENEBUILDER_H
#define COCOA_GALLIUM_BINDINGS_GLAMOR_SCENEBUILDER_H

#include <stack>

#include "Gallium/bindings/glamor/Exports.h"
#include "Glamor/Layers/Layer.h"
#include "Glamor/Layers/ContainerLayer.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class SceneBuilder
class SceneBuilder
{
public:
    SceneBuilder(int32_t width, int32_t height);
    ~SceneBuilder() = default;

    //! TSDecl: function build(): Scene
    v8::Local<v8::Value> build();

    //! TSDecl: function pop(): SceneBuilder
    v8::Local<v8::Value> pop();

    //! TSDecl: function pushOffset(x: number, y: number): SceneBuilder
    v8::Local<v8::Value> pushOffset(SkScalar x, SkScalar y);

    //! TSDecl: function addPicture(picture: CkPicture): SceneBuilder
    v8::Local<v8::Value> addPicture(v8::Local<v8::Value> picture, SkScalar dx, SkScalar dy);

    //! TSDecl: function addTexture(textureId: number,
    //!                             dx: number,
    //!                             dy: number,
    //!                             width: number,
    //!                             height: number,
    //!                             sampling: number): SceneBuilder
    v8::Local<v8::Value> addTexture(int64_t textureId,
                                    SkScalar dx,
                                    SkScalar dy,
                                    SkScalar width,
                                    SkScalar height,
                                    int32_t sampling);

private:
    v8::Local<v8::Object> getSelfHandle();
    void pushLayer(const std::shared_ptr<gl::ContainerLayer>& layer);
    void addLayer(const std::shared_ptr<gl::Layer>& layer);

    v8::Global<v8::Object> self_handle_;
    int32_t     width_;
    int32_t     height_;
    std::shared_ptr<gl::ContainerLayer> layer_tree_;
    std::stack<std::shared_ptr<gl::ContainerLayer>> layer_stack_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GALLIUM_BINDINGS_GLAMOR_SCENEBUILDER_H
