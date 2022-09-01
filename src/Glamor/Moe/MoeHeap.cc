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

#include "Glamor/Moe/MoeHeap.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkShader.h"
#include "include/core/SkBlender.h"

#include "Core/Errors.h"
GLAMOR_NAMESPACE_BEGIN

#define CLONE_TYPE(t) new (static_cast<void*>(&stored_[0])) t(*reinterpret_cast<const t*>(&lhs.stored_[0]))
MoeHeap::Cell::Cell(const Cell& lhs) noexcept
        : typeinfo_(lhs.typeinfo_)
        , stored_{}
{
    switch (typeinfo_)
    {
    case Typeinfo::kString:     CLONE_TYPE(SkString); break;
    case Typeinfo::kU32Array:   CLONE_TYPE(U32Array); break;
    case Typeinfo::kF32Array:   CLONE_TYPE(F32Array); break;
    case Typeinfo::kMatrix3x3:  CLONE_TYPE(SkMatrix); break;
    case Typeinfo::kMatrix4x4:  CLONE_TYPE(SkM44); break;
    case Typeinfo::kVector2:    CLONE_TYPE(SkV2);  break;
    case Typeinfo::kVector3:    CLONE_TYPE(SkV3);  break;
    case Typeinfo::kVector4:    CLONE_TYPE(SkV4);  break;
    case Typeinfo::kRect:       CLONE_TYPE(SkRect);  break;
    case Typeinfo::kRRect:      CLONE_TYPE(SkRRect); break;
    case Typeinfo::kRegion:     CLONE_TYPE(SkRegion); break;
    case Typeinfo::kPaint:      CLONE_TYPE(SkPaint); break;
    case Typeinfo::kPath:       CLONE_TYPE(SkPath); break;
    case Typeinfo::kSamplingOptions: CLONE_TYPE(SkSamplingOptions); break;
    case Typeinfo::kSpShader:   CLONE_TYPE(sk_sp<SkShader>); break;
    case Typeinfo::kSpBlender:  CLONE_TYPE(sk_sp<SkBlender>); break;
    case Typeinfo::kSpColorFilter:   CLONE_TYPE(sk_sp<SkColorFilter>); break;
    case Typeinfo::kSpImageFilter:   CLONE_TYPE(sk_sp<SkImageFilter>); break;
    case Typeinfo::kSpMaskFilter:    CLONE_TYPE(sk_sp<SkMaskFilter>); break;
    case Typeinfo::kSpPathEffect:    CLONE_TYPE(sk_sp<SkPathEffect>); break;

    case Typeinfo::kSpImage:         CLONE_TYPE(sk_sp<SkImage>); break;
    case Typeinfo::kSpPicture:       CLONE_TYPE(sk_sp<SkPicture>); break;
    case Typeinfo::kSpBitmap:        CLONE_TYPE(std::shared_ptr<SkBitmap>); break;
    default:
        MARK_UNREACHABLE("Invalid typeinfo for cell: could not clone object properly");
    }
}
#undef CLONE_TYPE

MoeHeap::Cell::~Cell()
{
#define DESTRUCT_TYPE(type) reinterpret_cast<type *>(&stored_[0])->~type()
#define DESTRUCT_TYPE2(type, dtor) reinterpret_cast<type *>(&stored_[0])->dtor()
    switch (typeinfo_)
    {
    case Typeinfo::kString:     DESTRUCT_TYPE(SkString); break;
    case Typeinfo::kU32Array:   DESTRUCT_TYPE(U32Array); break;
    case Typeinfo::kF32Array:   DESTRUCT_TYPE(F32Array); break;
    case Typeinfo::kMatrix3x3:  DESTRUCT_TYPE(SkMatrix); break;
    case Typeinfo::kMatrix4x4:  DESTRUCT_TYPE(SkM44); break;
    case Typeinfo::kVector2:    DESTRUCT_TYPE(SkV2); break;
    case Typeinfo::kVector3:    DESTRUCT_TYPE(SkV3); break;
    case Typeinfo::kVector4:    DESTRUCT_TYPE(SkV4); break;
    case Typeinfo::kRect:       DESTRUCT_TYPE(SkRect); break;
    case Typeinfo::kRRect:      DESTRUCT_TYPE(SkRRect); break;
    case Typeinfo::kRegion:     DESTRUCT_TYPE(SkRegion); break;
    case Typeinfo::kPaint:      DESTRUCT_TYPE(SkPaint); break;
    case Typeinfo::kPath:       DESTRUCT_TYPE(SkPath); break;
    case Typeinfo::kSamplingOptions: DESTRUCT_TYPE(SkSamplingOptions); break;
    case Typeinfo::kSpShader:   DESTRUCT_TYPE(sk_sp<SkShader>); break;
    case Typeinfo::kSpBlender:  DESTRUCT_TYPE(sk_sp<SkBlender>); break;
    case Typeinfo::kSpColorFilter:   DESTRUCT_TYPE(sk_sp<SkColorFilter>); break;
    case Typeinfo::kSpImageFilter:   DESTRUCT_TYPE(sk_sp<SkImageFilter>); break;
    case Typeinfo::kSpMaskFilter:    DESTRUCT_TYPE(sk_sp<SkMaskFilter>); break;
    case Typeinfo::kSpPathEffect:    DESTRUCT_TYPE(sk_sp<SkPathEffect>); break;

    case Typeinfo::kSpImage:         DESTRUCT_TYPE(sk_sp<SkImage>); break;
    case Typeinfo::kSpPicture:       DESTRUCT_TYPE(sk_sp<SkPicture>); break;
    case Typeinfo::kSpBitmap:        DESTRUCT_TYPE2(std::shared_ptr<SkBitmap>,
                                                    std::shared_ptr<SkBitmap>::~shared_ptr); break;
    default:
        MARK_UNREACHABLE("Invalid typeinfo for cell: could not destruct object properly");
    }
#undef DESTRUCT_TYPE
#undef DESTRUCT_TYPE2
}

MoeHeap::MoeHeap()
{
    profile_.heap_cell_size = sizeof(Cell);
}

bool MoeHeap::HasKey(uint32_t key)
{
    return cells_map_.count(key) > 0;
}

void MoeHeap::Clone(uint32_t from, uint32_t key)
{
    if (cells_map_.count(from) == 0)
        throw RuntimeException(__func__, fmt::format("Key {} points to an invalid heap object", from));
    if (cells_map_.count(key) > 0)
        throw RuntimeException(__func__, fmt::format("Key {} has been used for another heap object", key));
    cells_map_[key] = std::make_unique<Cell>(*cells_map_[from]);

    profile_.allocation_count++;
    profile_.heap_total_size += profile_.heap_cell_size;
    profile_.leaked_cells++;
}

void MoeHeap::Free(uint32_t key)
{
    if (cells_map_.count(key) == 0)
        throw RuntimeException(__func__, fmt::format("Key {} points to an invalid heap object", key));

    cells_map_.erase(key);
    profile_.leaked_cells--;
    profile_.heap_total_size -= profile_.heap_cell_size;
}

void MoeHeap::ProfileResult(Profile& out)
{
    std::memcpy(&out, &profile_, sizeof(Profile));
}


GLAMOR_NAMESPACE_END
