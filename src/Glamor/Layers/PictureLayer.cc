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

#include "Glamor/Layers/PictureLayer.h"
GLAMOR_NAMESPACE_BEGIN

PictureLayer::PictureLayer(const SkPoint& offset, const sk_sp<SkPicture>& picture)
    : sk_picture_(picture)
    , offset_(offset)
{
}

void PictureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SkRect bounds = sk_picture_->cullRect().makeOffset(offset_.x(), offset_.y());
    SetPaintBounds(bounds);
}

void PictureLayer::Paint(PaintContext *context) const
{
    SkAutoCanvasRestore scopedRestore(context->composed_canvas, true);
    context->composed_canvas->translate(offset_.x(), offset_.y());

    if (!context->paint)
    {
        // If there is no existing paint, send the drawing commands to
        // the canvas separately by `playback` method. Compared with `SkCanvas::drawPicture`,
        // this should be faster because if the paint is non-null,
        // Skia always draws the picture into a temporary layer before it
        // actually landing on the canvas.
        sk_picture_->playback(context->composed_canvas);
    }
    else
    {
        context->composed_canvas->drawPicture(sk_picture_, nullptr,
                                              context->paint);
    }
}

GLAMOR_NAMESPACE_END
