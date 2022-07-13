#ifndef COCOA_GLAMOR_FRAMEGENERATORBASE_H
#define COCOA_GLAMOR_FRAMEGENERATORBASE_H

#include "include/core/SkSurface.h"
#include "include/core/SkPicture.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class Blender;

class FrameGeneratorBase
{
public:
    explicit FrameGeneratorBase(const Shared<Blender>& blender)
        : weak_blender_(blender) {}
    virtual ~FrameGeneratorBase() = default;

    g_nodiscard g_inline Shared<Blender> GetBlender() const {
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
    Weak<Blender>        weak_blender_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_FRAMEGENERATORBASE_H
