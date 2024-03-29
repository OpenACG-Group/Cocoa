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

#ifndef COCOA_GLAMOR_LAYERS_BACKDROPFILTERLAYER_H
#define COCOA_GLAMOR_LAYERS_BACKDROPFILTERLAYER_H

#include "include/core/SkImageFilter.h"
#include "include/core/SkBlendMode.h"

#include "Glamor/Layers/Layer.h"
#include "Glamor/Layers/ContainerLayer.h"
GLAMOR_NAMESPACE_BEGIN

class BackdropFilterLayer : public ContainerLayer
{
public:
    BackdropFilterLayer(const sk_sp<SkImageFilter>& filter,
                        SkBlendMode blend_mode, bool auto_child_clip);
    ~BackdropFilterLayer() override = default;

    void Preroll(PrerollContext *context, const SkMatrix &matrix) override;
    void Paint(PaintContext *context) override;
    void ToString(std::ostream& out) override;

    ContainerAttributeChanged OnContainerDiffUpdateAttributes(
            const std::shared_ptr<ContainerLayer>& other) override;

    const char *GetLayerTypeName() override {
        return "BackdropFilterLayer";
    }

private:
    bool                    auto_child_clip_;
    sk_sp<SkImageFilter>    image_filter_;
    SkBlendMode             blend_mode_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_BACKDROPFILTERLAYER_H
