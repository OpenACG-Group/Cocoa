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
#include "include/core/SkCanvas.h"

#include "Glamor/RasterFrameGenerator.h"
GLAMOR_NAMESPACE_BEGIN

RasterFrameGenerator::RasterFrameGenerator(const std::shared_ptr<ContentAggregator>& blender)
    : FrameGeneratorBase(blender)
{
}

RasterFrameGenerator::~RasterFrameGenerator() = default;

void RasterFrameGenerator::OnPaint(SkSurface *surface, const sk_sp<SkPicture>& picture,
                                   const SkIRect& rect)
{
    SkCanvas *canvas = surface->getCanvas();
    SkAutoCanvasRestore autoRestore(canvas, true);

    canvas->clipIRect(rect);
    picture->playback(canvas);
}

GLAMOR_NAMESPACE_END
