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

#include "Core/TraceEvent.h"
#include "Gallium/bindings/renderer/ImageInfo.h"
#include "Gallium/bindings/present/ContentAggregator.h"
#include "Gallium/bindings/present/Promisify.h"
#include "Gallium/bindings/present/Scene.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

ContentAggregator::ContentAggregator(v8::Global<v8::Object> surface, std::shared_ptr<gl::ContentAggregator> CA)
    : surface_(std::move(surface)), CA_(std::move(CA))
{
    // TODO(present:sora): handle signals
}

void ContentAggregator::NotifyParentSurfaceDispose()
{
    NotifyDisposeState(DisposeState::kDisposed);
}

ffi::RetLocal<v8::Value> ContentAggregator::requestImageInfo()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteTask::Submit<SkImageInfo>(isolate, [CA = CA_]() {
        SkISize size = SkISize::Make(CA->GetWidth(), CA->GetHeight());
        return SkImageInfo::Make(size, CA->GetOutputColorInfo());
    }, [isolate](const SkImageInfo& info) {
        return ffi::JSObject::New<renderer::ImageInfo>(isolate, info);
    });
}

ffi::RetLocal<v8::Value> ContentAggregator::purgeRasterCacheResources()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return PromisifiedRemoteCall::Call(isolate, CA_, {}, GLOP_CONTENTAGGREGATOR_PURGE_RASTER_CACHE_RESOURCES);
}

ffi::RetLocal<v8::Value> ContentAggregator::update(const ffi::Class<Scene>& scene)
{
    TRACE_EVENT("present", "ContentAggregator::update");

    using UpdateResult = gl::ContentAggregator::UpdateResult;
    return PromisifiedRemoteCall::Call(
        v8::Isolate::GetCurrent(),
        CA_,
        PromisifiedRemoteCall::GenericConvert<AutoEnumCast<UpdateResult>>,
        GLOP_CONTENTAGGREGATOR_UPDATE,
        std::shared_ptr<gl::LayerTree>(scene->TakeLayerTree())
    );
}

GALLIUM_BINDINGS_PRESENT_NS_END
