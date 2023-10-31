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

#include "include/core/SkPicture.h"
#include "include/core/SkImage.h"

#include "fmt/format.h"

#include "Glamor/Layers/PictureLayer.h"
#include "Glamor/Layers/LayerGenerationCache.h"
GLAMOR_NAMESPACE_BEGIN

PictureLayer::PictureLayer(bool auto_fast_clip, const sk_sp<SkPicture>& picture)
    : Layer(Type::kPicture)
    , sk_picture_(picture)
{
}

PictureLayer::~PictureLayer() = default;

void PictureLayer::DiffUpdate(const std::shared_ptr<Layer>& other)
{
    CHECK(other->GetType() == Type::kPicture);
    auto layer = std::static_pointer_cast<PictureLayer>(other);

    // TODO(sora): Do we need to deep-compare the picture?
    if (layer->sk_picture_->uniqueID() != sk_picture_->uniqueID())
        IncreaseGenerationId();

    sk_picture_ = layer->sk_picture_;
}

void PictureLayer::Preroll(PrerollContext *context, const SkMatrix& matrix)
{
    SetPaintBounds(sk_picture_->cullRect());
}

void PictureLayer::Paint(PaintContext *context)
{
    SkCanvas *canvas = context->multiplexer_canvas;
    CHECK(canvas);

    if (context->cache->TryDrawCacheImageSnapshot(this, context))
        return;

    SkAutoCanvasRestore canvas_restore(canvas, true);
    canvas->clipRect(sk_picture_->cullRect());
    canvas->drawPicture(sk_picture_, nullptr, context->GetCurrentPaintPtr());
}

void PictureLayer::ToString(std::ostream& out)
{
    SkRect bounds = sk_picture_->cullRect();
    out << fmt::format("(picture#{}:{} '(bounds {} {} {} {}) '(id {}))",
                       GetUniqueId(), GetGenerationId(),
                       bounds.x(), bounds.y(), bounds.width(), bounds.height(),
                       sk_picture_->uniqueID());
}

GLAMOR_NAMESPACE_END
