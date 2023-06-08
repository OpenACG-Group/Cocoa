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

#include <chrono>

#include "fmt/format.h"

#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkDeferredDisplayListRecorder.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"

#include "Core/StandaloneThreadPool.h"

#include "Glamor/HWComposeTileFrameGenerator.h"
#include "Glamor/HWComposeSwapchain.h"
#include "Glamor/Surface.h"
#include "Glamor/Blender.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

std::vector<SkIRect> compute_tile_clips(int32_t width, int32_t height)
{
    CHECK(width > 0 && height > 0);

    int32_t tileWidth = GlobalScope::Ref().GetOptions().GetTileWidth();
    int32_t tileHeight = GlobalScope::Ref().GetOptions().GetTileHeight();
    CHECK(tileWidth > 0 && tileHeight > 0);

    int32_t xTiles = width / tileWidth;
    int32_t yTiles = height / tileHeight;
    int32_t xPadding = width - xTiles * tileWidth;
    int32_t yPadding = height - yTiles * tileHeight;

    int32_t xActualTiles = xTiles + ((xPadding == 0) ? 0 : 1);
    int32_t yActualTiles = yTiles + ((yPadding == 0) ? 0 : 1);

    std::vector<SkIRect> result(xActualTiles * yActualTiles);
    for (int32_t ty = 0; ty < yTiles; ty++)
    {
        for (int32_t tx = 0; tx < xTiles; tx++)
        {
            result[ty * xActualTiles + tx].setXYWH(tx * tileWidth, ty * tileHeight,
                                                   tileWidth, tileHeight);
        }
    }

    // Computing padding rectangles
    if (yPadding > 0)
    {
        for (int32_t tx = 0; tx < xTiles; tx++)
        {
            result[yTiles * xActualTiles + tx].setXYWH(tx * tileWidth, yTiles * tileHeight,
                                                       tileWidth, yPadding);
        }
    }
    if (xPadding > 0)
    {
        for (int32_t ty = 0; ty < yTiles; ty++)
        {
            result[ty * xActualTiles + xTiles].setXYWH(xTiles * tileWidth, ty * tileHeight,
                                                       xPadding, tileHeight);
        }
    }
    if (xPadding * yPadding > 0)
    {
        result[yTiles * xActualTiles + xTiles].setXYWH(xTiles * tileWidth, yTiles * tileHeight,
                                                       xPadding, yPadding);
    }

    return result;
}

void paint_tile_boundary(SkCanvas *canvas, const SkIRect& rect)
{
    SkRect R = SkRect::Make(rect);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setStroke(true);
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);
    canvas->drawRect(R, paint);

    SkPath path;

    static constexpr SkScalar P = 10.0;
    static constexpr SkVector d[8] = {
        {0,  P}, { P, 0},
        {0,  P}, {-P, 0},
        {0, -P}, {-P, 0},
        {0, -P}, { P, 0}
    };

    SkVector v[4];
    R.toQuad(v);

    for (int i = 0; i < 4; i++)
    {
        path.moveTo(v[i] + d[2 * i]);
        path.lineTo(v[i]);
        path.lineTo(v[i] + d[2 * i + 1]);
    }

    paint.setColor(SK_ColorBLUE);
    paint.setStrokeWidth(2);
    canvas->drawPath(path, paint);
}

} // namespace anonymous

HWComposeTileFrameGenerator::HWComposeTileFrameGenerator(const Shared<Blender>& blender)
    : FrameGeneratorBase(blender)
{
    std::vector<SkIRect> clips = compute_tile_clips(blender->GetWidth(),
                                                    blender->GetHeight());

    Shared<RenderTarget> rt = blender->GetOutputSurface()->GetRenderTarget();
    CHECK(rt);

    Shared<HWComposeSwapchain> swapchain = rt->GetHWComposeSwapchain();
    CHECK(swapchain);
    sk_sp<GrDirectContext> directCtx = swapchain->GetSkiaDirectContext();
    CHECK(directCtx);

    for (const SkIRect& clip : clips)
    {

        SkImageInfo textureImageInfo = SkImageInfo::Make(SkISize::Make(clip.width(), clip.height()),
                                                         blender->GetOutputColorInfo());
        sk_sp<SkSurface> texture = SkSurfaces::RenderTarget(directCtx.get(),
                                                            skgpu::Budgeted::kNo,
                                                            textureImageInfo);

        if (!texture)
            throw RuntimeException(__func__, "Failed to create texture for tiles");

        tile_blocks_.emplace_back(TileBlock {
            .tile_rect = clip,
            .backend_texture = texture
        });
    }
}

void HWComposeTileFrameGenerator::OnPaint(SkSurface *surface,
                                          const sk_sp<SkPicture>& picture,
                                          const SkIRect& clip)
{
    auto& threadpool = GlobalScope::Ref().GetRenderWorkersThreadPool();
    bool showTileBoundaries = GlobalScope::Ref().GetOptions().GetShowTileBoundaries();

    // SkAutoCanvasRestore restore(surface->getCanvas(), true);
    // surface->getCanvas()->clipIRect(clip);
    // surface->getCanvas()->drawPicture(picture);

    for (TileBlock& tb : tile_blocks_)
    {
        SkIRect innerTileClip = SkIRect::MakeEmpty();
        if (!innerTileClip.intersect(tb.tile_rect, clip))
            continue;

        SkSurfaceCharacterization characterization;
        tb.backend_texture->characterize(&characterization);

        SkIRect tileSpaceInnerClip = SkIRect::MakeXYWH(innerTileClip.x() - tb.tile_rect.x(),
                                                       innerTileClip.y() - tb.tile_rect.y(),
                                                       innerTileClip.width(),
                                                       innerTileClip.height());

        auto offsetX = static_cast<SkScalar>(tb.tile_rect.x());
        auto offsetY = static_cast<SkScalar>(tb.tile_rect.y());
        tb.future_dlist = threadpool->enqueue([picture, tileSpaceInnerClip, offsetX, offsetY, characterization]() {
            SkDeferredDisplayListRecorder recorder(characterization);
            SkCanvas *canvas = recorder.getCanvas();

            SkAutoCanvasRestore restore(canvas, true);
            canvas->clipIRect(tileSpaceInnerClip);
            canvas->translate(-offsetX, -offsetY);
            picture->playback(canvas);

            return recorder.detach();
        });
    }

    SkCanvas *screenCanvas = surface->getCanvas();
    SkAutoCanvasRestore restoreScope(screenCanvas, true);

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);

    for (TileBlock& tb : tile_blocks_)
    {
        if (tb.future_dlist.has_value())
        {
            sk_sp<SkDeferredDisplayList> list = tb.future_dlist->get();
            tb.future_dlist.reset();
            CHECK(tb.backend_texture->draw(list, 0, 0));
        }

        tb.backend_texture->draw(screenCanvas,
                                 static_cast<SkScalar>(tb.tile_rect.x()),
                                 static_cast<SkScalar>(tb.tile_rect.y()),
                                 SkSamplingOptions(),
                                 &paint);
    }

    if (showTileBoundaries)
    {
        for (const TileBlock& tb : tile_blocks_)
            paint_tile_boundary(screenCanvas, tb.tile_rect);
    }
}

GLAMOR_NAMESPACE_END
