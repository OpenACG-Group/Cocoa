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

#ifndef COCOA_GLAMOR_LAYERS_PICTURELAYER_H
#define COCOA_GLAMOR_LAYERS_PICTURELAYER_H

#include "include/core/SkPicture.h"

#include "Glamor/Layers/Layer.h"
#include "Glamor/MaybeGpuObject.h"
GLAMOR_NAMESPACE_BEGIN

class PictureLayer : public Layer
{
public:
    PictureLayer(const SkPoint& offset, const sk_sp<SkPicture>& picture);
    ~PictureLayer() override = default;

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;
    void Paint(PaintContext *context) const override;

private:
    sk_sp<SkPicture> sk_picture_;

    // top-left corner of picture in parent's coordinate
    SkPoint offset_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_PICTURELAYER_H
