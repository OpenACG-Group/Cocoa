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

#ifndef COCOA_GLAMOR_HWCOMPOSEOFFSCREEN_H
#define COCOA_GLAMOR_HWCOMPOSEOFFSCREEN_H

#include <list>

#include "include/core/SkSurface.h"

#include "Glamor/Glamor.h"
#include "Glamor/SkiaGpuContextOwner.h"
#include "Glamor/HWComposeDevice.h"
GLAMOR_NAMESPACE_BEGIN

class HWComposeContext;

/**
 * Like `HWComposeSwapchain`, `HWComposeOffscreen` represents a
 * Skia GPU context, which can be used for rendering directly, with
 * an associated logical device. But `HWComposeOffscreen` does not
 * provide onscreen surfaces for onscreen rendering. Instead, it
 * supports creating any number of `SkSurface` for offscreen rendering.
 *
 * Drawing offscreen rendering results on a onscreen buffer requires
 * memory transferring between different GPU contexts, which can be
 * done with the help of `SkiaGpuContextOwner::TransferSkSurfaceFrom()`
 * without any memory copy (see `HWComposeDevice` for more details).
 *
 * `HWComposeOffscreen` can create `SkSurface` and other GPU-backed Skia
 * objects. GPU resources are reference-counted, and it is safe to delete
 * `HWComposeOffscreen` before all the created Skia objects are deleted.
 */
class HWComposeOffscreen : public SkiaGpuContextOwner
{
public:
    static std::unique_ptr<HWComposeOffscreen> Make(
            const std::shared_ptr<HWComposeContext>& context);

    HWComposeOffscreen();
    ~HWComposeOffscreen() override = default;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_HWCOMPOSEOFFSCREEN_H
