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

#ifndef COCOA_GLAMOR_LAYERS_RASTERCACHEKEY_H
#define COCOA_GLAMOR_LAYERS_RASTERCACHEKEY_H

#include <utility>
#include <vector>

#include "include/core/SkMatrix.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class RasterCacheLayerId
{
public:
    enum class Type
    {
        kPicture,
        kContainer
    };

    explicit RasterCacheLayerId(uint64_t picture_unique_id)
        : id_type_(Type::kPicture)
        , picture_unique_id_(picture_unique_id) {}
    explicit RasterCacheLayerId(std::vector<RasterCacheLayerId> child_ids)
        : id_type_(Type::kContainer)
        , picture_unique_id_(0)
        , child_ids_(std::move(child_ids)) {}

    g_nodiscard g_inline Type GetType() const {
        return id_type_;
    }

    g_nodiscard g_inline uint64_t GetHash() const {
        if (cached_hash_)
            return *cached_hash_;
        cached_hash_ = ComputeHashValue();
        return *cached_hash_;
    }

    g_nodiscard g_inline uint64_t GetPictureUniqueId() const {
        CHECK(id_type_ == Type::kPicture);
        return picture_unique_id_;
    }

    g_inline bool operator==(const RasterCacheLayerId& other) const {
        return (GetHash() == other.GetHash());
    }

private:
    g_nodiscard uint64_t ComputeHashValue() const;

    mutable std::optional<uint64_t>  cached_hash_;
    Type                             id_type_;
    uint64_t                         picture_unique_id_;
    std::vector<RasterCacheLayerId>  child_ids_;
};

class RasterCacheKey
{
public:
    struct Hasher
    {
        uint64_t operator()(const RasterCacheKey& key) const {
            return key.layer_id_.GetHash();
        }
    };

    struct EqComparator
    {
        bool operator()(const RasterCacheKey& a, const RasterCacheKey& b) const {
            return (a.layer_id_ == b.layer_id_ && a.matrix_ == b.matrix_);
        }
    };

    template<typename V>
    using Map = std::unordered_map<RasterCacheKey, V, Hasher, EqComparator>;

    RasterCacheKey(RasterCacheLayerId layer, const SkMatrix& matrix)
        : matrix_(matrix)
        , layer_id_(std::move(layer))
    {
        matrix_[SkMatrix::kMTransX] = 0;
        matrix_[SkMatrix::kMTransY] = 0;
    }

    g_nodiscard g_inline const SkMatrix& GetMatrix() const {
        return matrix_;
    }

    g_nodiscard g_inline const RasterCacheLayerId& GetLayerId() const {
        return layer_id_;
    }

private:
    SkMatrix            matrix_;
    RasterCacheLayerId  layer_id_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_RASTERCACHEKEY_H
