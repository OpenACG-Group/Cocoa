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

#ifndef COCOA_GLAMOR_LAYERS_RASTERDRAWOPOBSERVER_H
#define COCOA_GLAMOR_LAYERS_RASTERDRAWOPOBSERVER_H

#include "include/utils/SkNoDrawCanvas.h"

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class RasterDrawOpObserver : public SkCanvasVirtualEnforcer<SkNoDrawCanvas>
{
public:
    RasterDrawOpObserver(int32_t width, int32_t height);
    explicit RasterDrawOpObserver(const SkIRect& cull);
    ~RasterDrawOpObserver() override = default;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_LAYERS_RASTERDRAWOPOBSERVER_H
