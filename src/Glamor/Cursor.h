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

#ifndef COCOA_GLAMOR_CURSOR_H
#define COCOA_GLAMOR_CURSOR_H

#include "include/core/SkPoint.h"

#include "Glamor/Glamor.h"
#include "Glamor/PresentRemoteHandle.h"
GLAMOR_NAMESPACE_BEGIN

#define GLOP_CURSOR_DISPOSE                 1
#define GLOP_CURSOR_GET_HOTSPOT_VECTOR      2

class CursorTheme;

/**
 * Basically `Cursor` is a highly platform-specific class.
 */
class Cursor : public PresentRemoteHandle
{
public:
    explicit Cursor(std::weak_ptr<CursorTheme> theme = {});
    ~Cursor() override;

    g_async_api void Dispose();
    g_async_api SkIVector GetHotspotVector();

    g_private_api bool HasAnimation();
    g_private_api void TryAbortAnimation();
    g_private_api void TryStartAnimation();

private:
    virtual void OnDispose() = 0;
    virtual SkIVector OnGetHotspotVector() = 0;
    virtual bool OnHasAnimation() = 0;
    virtual void OnTryAbortAnimation() = 0;
    virtual void OnTryStartAnimation() = 0;

    bool                        disposed_;
    // May be nullptr if the cursor is not loaded from a theme
    std::weak_ptr<CursorTheme>  theme_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_CURSOR_H
