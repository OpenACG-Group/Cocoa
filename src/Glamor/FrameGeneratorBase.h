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

#ifndef COCOA_GLAMOR_FRAMEGENERATORBASE_H
#define COCOA_GLAMOR_FRAMEGENERATORBASE_H

#include "include/core/SkSurface.h"
#include "include/core/SkPicture.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class ContentAggregator;

class FrameGeneratorBase
{
public:
    explicit FrameGeneratorBase(const Shared<ContentAggregator>& blender)
        : weak_blender_(blender) {}
    virtual ~FrameGeneratorBase() = default;

    g_nodiscard g_inline Shared<ContentAggregator> GetBlender() const {
        CHECK(!weak_blender_.expired());
        return weak_blender_.lock();
    }

    g_inline void Paint(SkSurface *surface, const sk_sp<SkPicture>& picture, const SkIRect& rect) {
        CHECK(surface && picture);
        this->OnPaint(surface, picture, rect);
    }

protected:
    virtual void OnPaint(SkSurface *surface,
                         const sk_sp<SkPicture>& picture, const SkIRect& rect) = 0;

private:
    std::weak_ptr<ContentAggregator>        weak_blender_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_FRAMEGENERATORBASE_H
