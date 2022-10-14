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

#ifndef COCOA_GLAMOR_LAYERS_TEXTURELAYER_H
#define COCOA_GLAMOR_LAYERS_TEXTURELAYER_H

#include "include/core/SkPoint.h"
#include "include/core/SkSamplingOptions.h"

#include "Glamor/Glamor.h"
#include "Glamor/Layers/Layer.h"
GLAMOR_NAMESPACE_BEGIN

class TextureLayer : public Layer
{
public:
    TextureLayer(int64_t texture_id,
                 const SkPoint& offset,
                 const SkISize& size,
                 const SkSamplingOptions& sampling);
    ~TextureLayer() override = default;

    void Preroll(PrerollContext *context, const SkMatrix& matrix) override;
    void Paint(PaintContext *context) const override;
    void ToString(std::ostream& out) override;

private:
    int64_t                 texture_id_;
    // Top-left corner of texture in the parent's coordinate
    SkPoint                 offset_;
    // Rescale texture to fit `size_`, using `sampling_options_`
    SkISize                 size_;
    SkSamplingOptions       sampling_options_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_TEXTURELAYER_H
