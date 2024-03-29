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

#include "include/core/SkPicture.h"

#include "Core/EventLoop.h"
#include "Core/Journal.h"
#include "Gallium/bindings/glamor/Scene.h"
#include "Gallium/bindings/glamor/CkImageWrap.h"
#include "Glamor/Layers/LayerTree.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Gallium.bindings.Glamor)

Scene::Scene(const std::shared_ptr<gl::ContainerLayer>& rootLayer, const SkISize& frameSize)
{
    layer_tree_ = std::make_unique<gl::LayerTree>(frameSize);
    layer_tree_->SetRootLayer(rootLayer);
}

Scene::~Scene() = default;

std::string Scene::toString()
{
    if (!layer_tree_)
        g_throw(Error, "Scene has been disposed");
    return layer_tree_->ToString();
}

v8::Local<v8::Value> Scene::toImage(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (layer_tree_ == nullptr)
        g_throw(Error, "No layer tree was associated with current Scene");

    // Flatten the layer tree and do an asynchronized rasterization.
    SkRect bounds = SkRect::MakeWH(static_cast<SkScalar>(width), static_cast<SkScalar>(height));
    gl::MaybeGpuObject<SkPicture> picture = layer_tree_->Flatten(bounds);

    if (picture == nullptr)
        g_throw(Error, "Failed in flattening layer tree to generate a SkPicture recording");

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

    auto global_resolver = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

    // Submit a task to thread pool
    EventLoop::GetCurrent()->enqueueThreadPoolTask<sk_sp<SkImage>>([picture]() -> sk_sp<SkImage> {
        SkISize picture_size = SkISize::Make(static_cast<int32_t>(picture->cullRect().width()),
                                             static_cast<int32_t>(picture->cullRect().height()));
        SkImageInfo info = SkImageInfo::Make(picture_size,
                                             SkColorType::kN32_SkColorType,
                                             SkAlphaType::kPremul_SkAlphaType);

        sk_sp<SkSurface> surface = SkSurfaces::Raster(info);
        if (surface == nullptr)
            return nullptr;
        picture->playback(surface->getCanvas());
        return surface->makeImageSnapshot();

    }, [global_resolver, isolate](sk_sp<SkImage>&& image) {

        v8::HandleScope handleScope(isolate);
        v8::Local<v8::Context> context = isolate->GetCurrentContext();
        v8::Local<v8::Promise::Resolver> resolver = global_resolver->Get(isolate);

        if (image == nullptr)
        {
            resolver->Reject(context, binder::to_v8(isolate, "Rasterizer failed to draw layer tree")).Check();
        }
        else
        {
            resolver->Resolve(context, binder::NewObject<CkImageWrap>(isolate, image)).Check();
        }
        global_resolver->Reset();
    });

    return resolver->GetPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
