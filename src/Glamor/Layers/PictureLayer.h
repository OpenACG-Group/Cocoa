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
    PictureLayer(bool auto_fast_clip, const sk_sp<SkPicture>& picture);
    ~PictureLayer() override;

    void DiffUpdate(const std::shared_ptr<Layer>& other) override;

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;
    void Paint(PaintContext *context) override;
    void ToString(std::ostream& out) override;

    const char *GetLayerTypeName() override {
        return "PictureLayer";
    }

private:
    sk_sp<SkPicture> sk_picture_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_PICTURELAYER_H
