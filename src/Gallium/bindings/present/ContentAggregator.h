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

#ifndef COCOA_GALLIUM_BINDINGS_PRESENT_CONTENTAGGREGATOR_H
#define COCOA_GALLIUM_BINDINGS_PRESENT_CONTENTAGGREGATOR_H

#include "Glamor/ContentAggregator.h"
#include "Glamor/MaybeGpuObject.h"

#include "Gallium/ffi/Class.h"
#include "Gallium/bindings/EventEmitter.h"
#include "Gallium/bindings/present/Types.h"
GALLIUM_BINDINGS_PRESENT_NS_BEGIN

class Scene;

//! TSDecl: @class @nonconstructible @extends(@import(event) EventEmitterBase) ContentAggregator
class ContentAggregator : public EventEmitterBase
{
public:
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    ContentAggregator(v8::Global<v8::Object> surface, std::shared_ptr<gl::ContentAggregator> CA);
    ~ContentAggregator() override = default;

    void NotifyParentSurfaceDispose();

    //! TSDecl: @method requestImageInfo(): @promise(@import(renderer) ImageInfo)
    ffi::RetLocal<v8::Value> requestImageInfo();

    //! TSDecl: @method purgeRasterCacheResources(): @promise(void)
    ffi::RetLocal<v8::Value> purgeRasterCacheResources();

    //! TSDecl: @method update(scene: Scene): @promise(UpdateResult)
    ffi::RetLocal<v8::Value> update(const ffi::Class<Scene>& scene);

private:
    v8::Global<v8::Object>                  surface_;
    std::shared_ptr<gl::ContentAggregator>  CA_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_PRESENT_NS_END
#endif //COCOA_GALLIUM_BINDINGS_PRESENT_CONTENTAGGREGATOR_H
