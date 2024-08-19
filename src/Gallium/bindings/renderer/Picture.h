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

#ifndef COCOA_GALLIUM_BINDINGS_RENDERER_PICTURE_H
#define COCOA_GALLIUM_BINDINGS_RENDERER_PICTURE_H

#include "include/core/SkPicture.h"

#include "Gallium/ffi/Class.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/renderer/Types.h"
#include "Gallium/bindings/renderer/Flattenable.h"
#include "Gallium/bindings/renderer/Rect.h"
#include "Gallium/bindings/renderer/Matrix.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

class Canvas;

//! TSDecl: @class @nonconstructible @extends(Flattenable) Picture
class Picture : public Flattenable
{
public:
    // SkPicture is not thread-safe, thus `Picture` object is not transferable
    // to avoid problems caused by multithreading. Always use deserialize/serialize
    // mechanism to pass SkPicture between threads.
    // The toughest problem of sharing `SkPicture` among threads happens when the
    // SkPicture contains references to other objects (shaders, effects, etc.). Passing
    // such an SkPicture to another thread also means passing all the references that
    // the SkPicture retains to another thread. If those objects retain GPU resources,
    // sharing among threads means they may be destructed on any other thread, especially
    // on a thread different from where they are originally created. That may cause fatal
    // errors related to resource management, and may crash the whole program.
    FFI_JSOBJECT_ATTRS(ffi::ClassMetadata::kNone_Attr)

    explicit Picture(sk_sp<SkPicture> picture) : picture_(std::move(picture)) {}
    ~Picture() override = default;

    g_nodiscard const sk_sp<SkPicture>& GetSkPicture() const {
        return picture_;
    }

    //! TSDecl: @method @static Deserialize(memory: @mem(u8),
    //! TSDecl:                             deserializers: @union(Deserializers, null))
    //! TSDecl:                             : @union(null, Picture)
    FLATTENABLE_IMPL_DESERIALIZE(Picture)

    //! TSDecl: @method @static MakePlaceholder(cull: Rect): Picture
    static ffi::RetLocal<v8::Value> MakePlaceholder(const RectAdapter& cull);

    //! TSDecl: @method playback(canvas: Canvas): void
    ffi::Ret<void> playback(const ffi::Class<Canvas>& canvas);

    //! TSDecl: @property @readonly cullRect: Rect
    ffi::RetLocal<v8::Value> getCullRect();

    //! TSDecl: @property @readonly uniqueID: u32
    ffi::Ret<uint32_t> getUniqueID() {
        return picture_->uniqueID();
    }

    //! TSDecl: @method approximateOpCount(nested: boolean): i32
    ffi::Ret<int32_t> approximateOpCount(bool nested) {
        return picture_->approximateOpCount(nested);
    }

    //! TSDecl: @method approximateBytesUsed(): u64
    ffi::Ret<size_t> approximateBytesUsed() {
        return picture_->approximateBytesUsed();
    }

    //! TSDecl: @method makeShader(tmx: TileMode, tmy: TileMode, mode: FilterMode,
    //! TSDecl:                    localMatrix: @union(null, Mat3x3), tileRect: @union(null, Rect)): Shader
    ffi::RetLocal<v8::Value> makeShader(const ffi::Enum<SkTileMode>& tmx,
                                        const ffi::Enum<SkTileMode>& tmy,
                                        const ffi::Enum<SkFilterMode>& mode,
                                        const ffi::Opt<Mat3x3Adapter>& local_matrix,
                                        const ffi::Opt<RectAdapter>& tile_rect);

private:
    sk_sp<SkData> OnSerializeImpl(SkSerialProcs *procs) override;

    sk_sp<SkPicture> picture_;
    v8::Global<v8::Object> cache_cull_rect_;
};
//! TSDecl: @end

GALLIUM_BINDINGS_RENDERER_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RENDERER_PICTURE_H
