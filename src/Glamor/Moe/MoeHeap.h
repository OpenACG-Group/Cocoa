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

#ifndef COCOA_GLAMOR_MOE_MOEHEAP_H
#define COCOA_GLAMOR_MOE_MOEHEAP_H

#include "include/core/SkString.h"
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkPath.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkPicture.h"

#include "fmt/format.h"

#include "Core/Exception.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class MoeHeap
{
public:
    using U32Array = std::vector<uint32_t>;
    using F32Array = std::vector<float>;

    struct Cell
    {
        enum class Typeinfo
        {
            kMismatch,
            kString,
            kU32Array,
            kF32Array,
            kMatrix3x3,
            kMatrix4x4,
            kVector2,
            kVector3,
            kVector4,
            kRect,
            kRRect,
            kRegion,
            kPaint,
            kPath,
            kSamplingOptions,
            kSpShader,
            kSpBlender,
            kSpColorFilter,
            kSpImageFilter,
            kSpMaskFilter,
            kSpPathEffect,

            // These types cannot be allocated on heap by IR directly,
            // but they can be bound on heap before IR execution.
            kSpBitmap,
            kSpImage,
            kSpPicture
        };

        union ObjectSizeMeasurerUnion
        {
            g_maybe_unused SkString v_string;
            g_maybe_unused U32Array v_u32_array;
            g_maybe_unused F32Array v_f32_array;
            g_maybe_unused SkMatrix v_matrix_3x3;
            g_maybe_unused SkM44 v_matrix_4x4;
            g_maybe_unused SkV2 v_vector2;
            g_maybe_unused SkV3 v_vector3;
            g_maybe_unused SkV4 v_vector4;
            g_maybe_unused SkRect v_rect;
            g_maybe_unused SkRRect v_rrect;
            g_maybe_unused SkRegion v_region;
            g_maybe_unused SkPaint v_paint;
            g_maybe_unused SkPath v_path;
            g_maybe_unused SkSamplingOptions v_sampling_options;
            g_maybe_unused sk_sp<SkShader> v_sp_shader;
            g_maybe_unused sk_sp<SkBlender> v_sp_blender;
            g_maybe_unused sk_sp<SkColorFilter> v_sp_color_filter;
            g_maybe_unused sk_sp<SkImageFilter> v_sp_image_filter;
            g_maybe_unused sk_sp<SkMaskFilter> v_sp_mask_filter;
            g_maybe_unused sk_sp<SkPathEffect> v_sp_path_effect;

            g_maybe_unused Shared<SkBitmap> v_sp_bitmap;
            g_maybe_unused sk_sp<SkImage> v_sp_image;
            g_maybe_unused sk_sp<SkPicture> v_sp_picture;
        };

        template<typename T, typename...ArgsT>
        explicit Cell(T *placeholder, ArgsT&&...args);
        Cell(const Cell& lhs) noexcept;

        ~Cell();

        Typeinfo        typeinfo_;
        uint8_t         stored_[sizeof(ObjectSizeMeasurerUnion)];
    };

    struct Profile
    {
        size_t      heap_total_size = 0;
        size_t      heap_cell_size = 0;
        uint32_t    allocation_count = 0;
        uint32_t    extraction_count = 0;
        size_t      leaked_cells = 0;
    };

    MoeHeap();
    ~MoeHeap() = default;

    template<typename T, typename...ArgsT>
    void Allocate(uint32_t key, ArgsT&&...args);
    
    template<typename T>
    T& Extract(uint32_t key);
    
    void Clone(uint32_t from, uint32_t key);
    void Free(uint32_t key);
    void ProfileResult(Profile& out);
    bool HasKey(uint32_t key);

private:
    std::unordered_map<uint32_t, Unique<Cell>> cells_map_;
    Profile profile_;
};

namespace moe_details {

template<typename T>
g_inline constexpr MoeHeap::Cell::Typeinfo typeinfo_from_type()
{
    return MoeHeap::Cell::Typeinfo::kMismatch;
}

#define GEN_TYPEINFO_FROM_TYPE(type, typeinfo)                  \
template<>                                                      \
constexpr MoeHeap::Cell::Typeinfo typeinfo_from_type<type>() {  \
    return MoeHeap::Cell::Typeinfo::k##typeinfo;                \
}

GEN_TYPEINFO_FROM_TYPE(SkString, String)
GEN_TYPEINFO_FROM_TYPE(MoeHeap::U32Array, U32Array)
GEN_TYPEINFO_FROM_TYPE(MoeHeap::F32Array, F32Array)
GEN_TYPEINFO_FROM_TYPE(SkMatrix, Matrix3x3)
GEN_TYPEINFO_FROM_TYPE(SkM44, Matrix4x4)
GEN_TYPEINFO_FROM_TYPE(SkV2, Vector2)
GEN_TYPEINFO_FROM_TYPE(SkV3, Vector3)
GEN_TYPEINFO_FROM_TYPE(SkV4, Vector4)
GEN_TYPEINFO_FROM_TYPE(SkRect, Rect)
GEN_TYPEINFO_FROM_TYPE(SkRRect, RRect)
GEN_TYPEINFO_FROM_TYPE(SkRegion, Region)
GEN_TYPEINFO_FROM_TYPE(SkPaint, Paint)
GEN_TYPEINFO_FROM_TYPE(SkPath, Path)
GEN_TYPEINFO_FROM_TYPE(SkSamplingOptions, SamplingOptions)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkShader>, SpShader)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkBlender>, SpBlender)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkColorFilter>, SpColorFilter)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkImageFilter>, SpImageFilter)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkMaskFilter>, SpMaskFilter)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkPathEffect>, SpPathEffect)
GEN_TYPEINFO_FROM_TYPE(Shared<SkBitmap>, SpBitmap)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkImage>, SpImage)
GEN_TYPEINFO_FROM_TYPE(sk_sp<SkPicture>, SpPicture)

#undef GEN_TYPEINFO_FROM_TYPE

} // namespace moe_details

template<typename T, typename...ArgsT>
g_inline MoeHeap::Cell::Cell(g_maybe_unused T *placeholder, ArgsT&& ...args)
: typeinfo_(moe_details::typeinfo_from_type<T>())
, stored_{}
{
    constexpr Typeinfo typeinfo = moe_details::typeinfo_from_type<T>();
    static_assert(typeinfo != Typeinfo::kMismatch, "typecheck: invalid type to construct Cell");
    new (static_cast<void*>(&stored_[0])) T(std::forward<ArgsT>(args)...);
}

template<typename T, typename...ArgsT>
g_inline void MoeHeap::Allocate(uint32_t key, ArgsT&& ...args)
{
    if (cells_map_.count(key) > 0)
        throw RuntimeException(__func__, fmt::format("Key {} has been used for another heap object", key));

    T *placeholder = nullptr;
    cells_map_.try_emplace(key, new Cell(placeholder, std::forward<ArgsT>(args)...));

    profile_.allocation_count++;
    profile_.heap_total_size += profile_.heap_cell_size;
    profile_.leaked_cells++;
}

template<typename T>
g_inline T& MoeHeap::Extract(uint32_t key)
{
    constexpr Cell::Typeinfo typeinfo = moe_details::typeinfo_from_type<T>();
    static_assert(typeinfo != MoeHeap::Cell::Typeinfo::kMismatch,
                  "typecheck: invalid type to extract from Cell");

    if (cells_map_.count(key) == 0)
        throw RuntimeException(__func__, fmt::format("Key {} points to an invalid heap object", key));
    Unique<Cell>& cell = cells_map_.at(key);

    profile_.extraction_count++;
    return *reinterpret_cast<T*>(&cell->stored_[0]);
}

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOE_MOEHEAP_H
