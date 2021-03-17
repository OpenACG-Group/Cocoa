#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPicture.h"
#include "include/core/SkCanvas.h"

#include "Core/Journal.h"
#include "Ciallo/Ciallo.h"
#include "Ciallo/Tree/PaintNode.h"
CIALLO_BEGIN_NS

PaintNode::ScopedPaint::ScopedPaint(PaintNode *paintNode)
    : fPaintNode(paintNode)
{
    if (fPaintNode != nullptr)
        fPaintNode->begin();
}

PaintNode::ScopedPaint::ScopedPaint(ScopedPaint&& that) noexcept
    : fPaintNode(that.fPaintNode)
{
    that.fPaintNode = nullptr;
}

PaintNode::ScopedPaint::~ScopedPaint()
{
    if (fPaintNode != nullptr)
        fPaintNode->finish();
}

// -------------------------------------------------------------------------

PaintNode *PaintNode::MakeFromParent(BaseNode *parent, int32_t width, int32_t height,
                                     int32_t x, int32_t y)
{
    return new PaintNode(parent, width, height, x, y);
}

PaintNode::PaintNode(BaseNode *parent, int32_t width, int32_t height,
                     int32_t x, int32_t y)
    : BaseNode(NodeKind::kPaintNode, parent),
      fWidth(width),
      fHeight(height),
      fLeft(x),
      fTop(y),
      fPicture(nullptr)
{
}

void PaintNode::resize(int32_t width, int32_t height)
{
    fWidth = width;
    fHeight = height;
}

void PaintNode::begin()
{
    if (fPictureRecorder.getRecordingCanvas() != nullptr)
        return;
    fPictureRecorder.beginRecording(fWidth, fHeight);
}

SkCanvas *PaintNode::asCanvas()
{
    return fPictureRecorder.getRecordingCanvas();
}

void PaintNode::finish()
{
    if (fPictureRecorder.getRecordingCanvas())
        fPicture = fPictureRecorder.finishRecordingAsPicture();
}

sk_sp<SkPicture> PaintNode::asPicture()
{
    return fPicture;
}

void PaintNode::moveTo(int32_t x, int32_t y)
{
    fLeft = x;
    fTop = y;
}

CIALLO_END_NS
