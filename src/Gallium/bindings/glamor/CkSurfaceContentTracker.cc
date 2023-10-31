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

#include "include/core/SkSurface.h"

#include "Glamor/Layers/GpuSurfaceViewLayer.h"
#include "Gallium/binder/Class.h"
#include "Gallium/bindings/glamor/CkSurfaceContentTracker.h"
#include "Gallium/bindings/glamor/CkSurfaceWrap.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkSurfaceContentTracker::CkSurfaceContentTracker(v8::Local<v8::Value> surface)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *wrap = binder::UnwrapObject<CkSurface>(isolate, surface);
    if (!wrap || wrap->isDisposed())
        g_throw(TypeError, "Argument `surface` must be a valid instance of CkSurface");
    tracker_ = std::make_unique<gl::GpuSurfaceViewLayer::ContentTracker>(wrap->GetSurface());
}

CkSurfaceContentTracker::CkSurfaceContentTracker(const gl::GpuSurfaceViewLayer::ContentTracker& other)
{
    tracker_ = std::make_unique<gl::GpuSurfaceViewLayer::ContentTracker>(other);
}

v8::Local<v8::Value> CkSurfaceContentTracker::fork()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::NewObject<CkSurfaceContentTracker>(isolate, *tracker_);
}

void CkSurfaceContentTracker::updateTrackPoint()
{
    tracker_->UpdateTrackPoint();
}

bool CkSurfaceContentTracker::hasChanged()
{
    return tracker_->HasChangedSinceLastTrackPoint();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
