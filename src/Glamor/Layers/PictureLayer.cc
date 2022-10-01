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

#include "fmt/format.h"

#include "Glamor/Layers/PictureLayer.h"
GLAMOR_NAMESPACE_BEGIN

PictureLayer::PictureLayer(const SkPoint& offset,
                           bool auto_fast_clip,
                           const sk_sp<SkPicture>& picture)
    : sk_picture_(picture)
    , offset_(offset)
    , auto_fast_clip_(auto_fast_clip)
{
}

PictureLayer::~PictureLayer() = default;

void PictureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect bounds = sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());
    SetPaintBounds(bounds);
}

void PictureLayer::Paint(PaintContext *context) const
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    SkAutoCanvasRestore canvas_restore(canvas, true);
    canvas->translate(offset_.x(), offset_.y());
    if (auto_fast_clip_)
        canvas->clipRect(sk_picture_->cullRect());

    canvas->drawPicture(sk_picture_, nullptr, context->GetCurrentPaintPtr());
}

GLAMOR_NAMESPACE_END
