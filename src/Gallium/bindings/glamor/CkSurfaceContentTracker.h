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

#ifndef COCOA_GLAMOR_LAYERS_CKSURFACECONTENTTRACKER_H
#define COCOA_GLAMOR_LAYERS_CKSURFACECONTENTTRACKER_H

#include "include/v8-value.h"

#include "Gallium/bindings/glamor/Types.h"
#include "Glamor/Layers/GpuSurfaceViewLayer.h"
#include "Gallium/bindings/ExportableObjectBase.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

//! TSDecl: class CkSurfaceContentTracker
class CkSurfaceContentTracker : public ExportableObjectBase
{
public:
    //! TSDecl: constructor(surface: CkSurface)
    explicit CkSurfaceContentTracker(v8::Local<v8::Value> surface);
    ~CkSurfaceContentTracker() = default;

    explicit CkSurfaceContentTracker(const gl::GpuSurfaceViewLayer::ContentTracker& other);

    g_nodiscard gl::GpuSurfaceViewLayer::ContentTracker *GetTracker() const {
        return tracker_.get();
    }

    //! TSDecl: function fork(): CkSurfaceContentTracker
    v8::Local<v8::Value> fork();

    //! TSDecl: function updateTrackPoint(): void
    void updateTrackPoint();

    //! TSDecl: function hasChanged(): boolean
    bool hasChanged();

private:
    std::unique_ptr<gl::GpuSurfaceViewLayer::ContentTracker> tracker_;
};

GALLIUM_BINDINGS_GLAMOR_NS_END
#endif //COCOA_GLAMOR_LAYERS_CKSURFACECONTENTTRACKER_H
