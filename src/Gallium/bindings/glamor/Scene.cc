#include "uv.h"

#include "include/core/SkPicture.h"

#include "Core/EventLoop.h"
#include "Core/Errors.h"
#include "Gallium/bindings/glamor/Scene.h"
#include "Glamor/Layers/LayerTree.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

Scene::Scene(const std::shared_ptr<glamor::ContainerLayer>& rootLayer,
             const SkISize& frameSize)
    : disposed_(false)
{
    layer_tree_ = std::make_unique<glamor::LayerTree>(frameSize);
    layer_tree_->SetRootLayer(rootLayer);
}

Scene::~Scene()
{
    CHECK(disposed_ && "Scene object must be disposed before destructing");
}

void Scene::dispose()
{
    // This function can be called for multiple times.
    if (disposed_)
        return;

    layer_tree_.reset();
    disposed_ = true;
}

v8::Local<v8::Value> Scene::toImage(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (layer_tree_ == nullptr)
        g_throw(Error, "No layer tree was associated with current Scene");

    // Flatten the layer tree and do a non-synchronized rasterization.
    SkRect bounds = SkRect::MakeWH(static_cast<SkScalar>(width), static_cast<SkScalar>(height));
    glamor::MaybeGpuObject<SkPicture> picture = layer_tree_->Flatten(bounds);

    if (picture == nullptr)
        g_throw(Error, "Failed in flattening layer tree to generate a SkPicture recording");

    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Promise::Resolver> resolver = v8::Promise::Resolver::New(context).ToLocalChecked();

    auto global_resolver = std::make_shared<v8::Global<v8::Promise::Resolver>>(isolate, resolver);

    // Submit a task to thread pool
    EventLoop::Ref().enqueueThreadPoolTask<sk_sp<SkImage>>([picture]() -> sk_sp<SkImage> {
        SkISize picture_size = SkISize::Make(static_cast<int32_t>(picture->cullRect().width()),
                                             static_cast<int32_t>(picture->cullRect().height()));
        SkImageInfo info = SkImageInfo::Make(picture_size,
                                             SkColorType::kN32_SkColorType,
                                             SkAlphaType::kPremul_SkAlphaType);

        sk_sp<SkSurface> surface = SkSurface::MakeRaster(info);
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
            resolver->Resolve(context, binder::Class<CkImageWrap>::create_object(isolate, image)).Check();
        }
        global_resolver->Reset();
    });

    return resolver->GetPromise();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
