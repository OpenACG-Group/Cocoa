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

#ifndef COCOA_GLAMOR_LAYERS_RECTCLIPLAYER_H
#define COCOA_GLAMOR_LAYERS_RECTCLIPLAYER_H

#include "Glamor/Layers/ContainerLayer.h"
#include "Glamor/Layers/ClippingLayerBase.h"
GLAMOR_NAMESPACE_BEGIN

class RectClipLayer : public ClippingLayerBase<SkRect>
{
public:
    RectClipLayer(const SkRect& rect, bool AA);
    ~RectClipLayer() override = default;

    ContainerAttributeChanged OnContainerDiffUpdateAttributes(
            const std::shared_ptr<ContainerLayer>& other) override;

    g_nodiscard SkRect OnGetClipShapeBounds() const override;
    void OnApplyClipShape(const SkRect& shape, PaintContext *ctx) const override;

    void ToString(std::ostream& os) override;

    const char *GetLayerTypeName() override {
        return "RectClipLayer";
    }

private:
    bool perform_anti_alias_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_RECTCLIPLAYER_H
