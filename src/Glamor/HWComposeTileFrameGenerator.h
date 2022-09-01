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

#ifndef COCOA_GLAMOR_HWCOMPOSETILEFRAMEGENERATOR_H
#define COCOA_GLAMOR_HWCOMPOSETILEFRAMEGENERATOR_H

#include <future>
#include <optional>

#include "include/core/SkDeferredDisplayList.h"
#include "include/core/SkSurface.h"
#include "include/core/SkImage.h"

#include "Glamor/Glamor.h"
#include "Glamor/FrameGeneratorBase.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeTileFrameGenerator : public FrameGeneratorBase
{
public:
    explicit HWComposeTileFrameGenerator(const Shared<Blender>& blender);
    ~HWComposeTileFrameGenerator() override = default;

    void OnPaint(SkSurface *surface, const sk_sp<SkPicture> &picture,
                 const SkIRect &rect) override;

private:
    struct TileBlock
    {
        using Future = std::future<sk_sp<SkDeferredDisplayList>>;

        SkIRect             tile_rect;
        sk_sp<SkSurface>    backend_texture;
        std::optional<Future> future_dlist;
    };

    std::vector<TileBlock>      tile_blocks_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_HWCOMPOSETILEFRAMEGENERATOR_H
