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

#include "Glamor/ContentAggregator.h"

#include "Core/TraceEvent.h"
#include "Gallium/binder/TypeTraits.h"
#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/PromiseHelper.h"
#include "Gallium/bindings/glamor/Scene.h"
#include "Gallium/bindings/glamor/CkImageWrap.h"
#include "Gallium/bindings/glamor/GpuDirectContext.h"
#include "Gallium/bindings/glamor/GpuExportedFd.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

ContentAggregatorWrap::ContentAggregatorWrap(std::shared_ptr<gl::PresentRemoteHandle> handle)
    : handle_(std::move(handle))
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    using PictCast = CreateObjCast<gl::MaybeGpuObject<SkPicture>, CriticalPictureWrap>;
    DefineSignalEventsOnEventEmitter(this, handle_, {
        { "picture-captured", GLSI_CONTENTAGGREGATOR_PICTURE_CAPTURED,
          GenericSignalArgsConverter<PictCast, NoCast<int32_t>> }
    });

    std::shared_ptr<gl::ContentAggregator> aggregator = handle_->As<gl::ContentAggregator>();
    if (aggregator->GetAttachedProfiler())
    {
        std::shared_ptr<gl::GProfiler> profiler = aggregator->GetAttachedProfiler();
        wrapped_profiler_.Reset(isolate,
            binder::NewObject<GProfilerWrap>(isolate, profiler));
    }
}

ContentAggregatorWrap::~ContentAggregatorWrap() = default;

v8::Local<v8::Value> ContentAggregatorWrap::getProfiler()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (wrapped_profiler_.IsEmpty())
        return v8::Null(isolate);

    return wrapped_profiler_.Get(isolate);
}

v8::Local<v8::Value> ContentAggregatorWrap::getNativeImageInfo()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto self = handle_->As<gl::ContentAggregator>();
    return PromisifiedRemoteTask::Submit<SkImageInfo>(
        isolate,
        [self]() {
            SkISize size = SkISize::Make(self->GetWidth(), self->GetHeight());
            return SkImageInfo::Make(size, self->GetOutputColorInfo());
        },
        [isolate](const SkImageInfo& info) {
            return binder::NewObject<CkImageInfo>(isolate, info);
        }
    );
}

v8::Local<v8::Value> ContentAggregatorWrap::dispose()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, handle_, {}, GLOP_CONTENTAGGREGATOR_DISPOSE);
}

v8::Local<v8::Value> ContentAggregatorWrap::update(v8::Local<v8::Value> sceneObject)
{
    TRACE_EVENT("main", "ContentAggregatorWrap::update");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    auto *scene = binder::UnwrapObject<Scene>(isolate, sceneObject);
    if (scene == nullptr)
        g_throw(TypeError, "Argument 'scene' must be an instance of Scene");

    std::shared_ptr<gl::LayerTree> layer_tree(scene->takeLayerTree());
    CHECK(layer_tree.unique());

    using UpdateResult = gl::ContentAggregator::UpdateResult;
    return PromisifiedRemoteCall::Call(
            isolate,
            handle_,
            PromisifiedRemoteCall::GenericConvert<AutoEnumCast<UpdateResult>>,
            GLOP_CONTENTAGGREGATOR_UPDATE,
            layer_tree
    );
}

v8::Local<v8::Value> ContentAggregatorWrap::captureNextFrameAsPicture()
{
    TRACE_EVENT("main", "ContentAggregatorWrap::captureNextFrameAsPicture");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, PromisifiedRemoteCall::GenericConvert<NoCast<int32_t>>,
            GLOP_CONTENTAGGREGATOR_CAPTURE_NEXT_FRAME_AS_PICTURE);
}

v8::Local<v8::Value> ContentAggregatorWrap::purgeRasterCacheResources()
{
    TRACE_EVENT("main", "ContentAggregatorWrap::purgeRasterCacheResources");
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_CONTENTAGGREGATOR_PURGE_RASTER_CACHE_RESOURCES);
}

v8::Local<v8::Value> ContentAggregatorWrap::importGpuSemaphoreFd(v8::Local<v8::Value> fd)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GpuExportedFd *handle = binder::UnwrapObject<GpuExportedFd>(isolate, fd);
    if (!handle || handle->isImportedOrClosed() ||
        handle->GetPayloadType() != GpuExportedFd::FdPayloadType::kSemaphore)
    {
        g_throw(TypeError, "Argument `fd` must be a valid GpuExportedFd");
    }
    int32_t fd_value = handle->CheckAndTakeDescriptor();
    return PromisifiedRemoteCall::Call(
        isolate,
        handle_,
        [](v8::Isolate *i, gl::PresentRemoteCallReturn& ret) {
            auto id = ret.GetReturnValue<gl::ContentAggregator::ImportedResourcesId>();
            return v8::BigInt::New(i, id);
        },
        GLOP_CONTENTAGGREGATOR_IMPORT_GPU_SEMAPHORE_FROM_FD,
        fd_value,
        true
    );
}

v8::Local<v8::Value> ContentAggregatorWrap::deleteImportedGpuSemaphore(v8::Local<v8::Value> id)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!id->IsBigInt())
        g_throw(TypeError, "Argument `id` must be a bigint");
    bool lossless;
    auto value = id.As<v8::BigInt>()->Int64Value(&lossless);
    if (!lossless)
        g_throw(RangeError, "Invalid id was provided by argument `id`");
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_CONTENTAGGREGATOR_DELETE_IMPORTED_GPU_SEMAPHORE, value);
}

v8::Local<v8::Value> ContentAggregatorWrap::importGpuCkSurface(v8::Local<v8::Value> fd)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    GpuExportedFd *handle = binder::UnwrapObject<GpuExportedFd>(isolate, fd);
    if (!handle || handle->isImportedOrClosed() ||
        handle->GetPayloadType() != GpuExportedFd::FdPayloadType::kSkSurface)
    {
        g_throw(TypeError, "Argument `fd` must be a valid GpuExportedFd");
    }
    auto payload_ptr = handle->GetPayload<GpuExportedFd::SkSurfacePayload>();
    GpuExportedFd::SkSurfacePayload payload_pod = *payload_ptr;
    handle->CheckAndTakeDescriptor();
    return PromisifiedRemoteCall::Call(
        isolate,
        handle_,
        [](v8::Isolate *i, gl::PresentRemoteCallReturn& ret) {
            auto id = ret.GetReturnValue<gl::ContentAggregator::ImportedResourcesId>();
            return v8::BigInt::New(i, id);
        },
        GLOP_CONTENTAGGREGATOR_IMPORT_GPU_SKSURFACE,
        payload_pod
    );
}

v8::Local<v8::Value> ContentAggregatorWrap::deleteImportedGpuCkSurface(v8::Local<v8::Value> id)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!id->IsBigInt())
        g_throw(TypeError, "Argument `id` must be a bigint");
    bool lossless;
    auto value = id.As<v8::BigInt>()->Int64Value(&lossless);
    if (!lossless)
        g_throw(RangeError, "Invalid id was provided by argument `id`");
    return PromisifiedRemoteCall::Call(
            isolate, handle_, {}, GLOP_CONTENTAGGREGATOR_DELETE_IMPORTED_GPU_SKSURFACE, value);
}

v8::Local<v8::Object> ContentAggregatorWrap::OnGetObjectSelf(v8::Isolate *isolate)
{
    return GetObjectWeakReference().Get(isolate);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
