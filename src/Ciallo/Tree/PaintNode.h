#ifndef COCOA_PAINTNODE_H
#define COCOA_PAINTNODE_H

#include <string>

#include "include/core/SkCanvas.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkVertices.h"
#include "include/core/SkShader.h"
#include "include/effects/SkRuntimeEffect.h"

#include "Ciallo/Ciallo.h"
#include "Ciallo/Tree/BaseNode.h"
CIALLO_BEGIN_NS

class PaintNode : public BaseNode
{
public:
    CIALLO_STATIC_NODE_KIND(NodeKind::kPaintNode)

    /**
     * The ScopedPaint class restricts drawing operations in the current scope.
     * When entering the scope (constructing ScopedPaint object), it calls the
     * PaintNode::begin method automatically.
     * When leaving the scope (destructing ScopedPaint object), it calls the
     * PaintNode::finish method automatically.
     * ScopedPaint objects cannot be copied, but move and reference are supported.
     */
    class ScopedPaint
    {
    public:
        explicit ScopedPaint(PaintNode *paintNode);
        ScopedPaint(ScopedPaint&& that) noexcept;
        /* ScopedPaint is non-copyable */
        ScopedPaint(const ScopedPaint& that) = delete;
        ~ScopedPaint();

    private:
        PaintNode *fPaintNode;
    };

    static PaintNode *MakeFromParent(BaseNode *parent,
                                     int32_t width,
                                     int32_t height,
                                     int32_t x,
                                     int32_t y);

    inline int32_t width() const    { return fWidth; }
    inline int32_t height() const   { return fHeight; }
    inline int32_t top() const      { return fTop; }
    inline int32_t left() const     { return fLeft; }

    /**
     * Note that it is only effective to call resize before the begin method.
     * Calling resize during the begin-finish process will take effect when
     * the next begin() is called.
     */
    void resize(int32_t width, int32_t height);
    void moveTo(int32_t x, int32_t y);

    void begin();
    void finish();

    SkCanvas *asCanvas();
    sk_sp<SkPicture> asPicture();

protected:
    PaintNode(BaseNode *parent, int32_t width, int32_t height,
              int32_t x, int32_t y);

private:
    int32_t                 fWidth;
    int32_t                 fHeight;
    int32_t                 fLeft;
    int32_t                 fTop;
    SkPictureRecorder       fPictureRecorder;
    sk_sp<SkPicture>        fPicture;
};

CIALLO_END_NS
#endif //COCOA_PAINTNODE_H
