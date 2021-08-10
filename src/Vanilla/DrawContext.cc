#include <cassert>

#include "Vanilla/DrawContext.h"
#include "Vanilla/Window.h"
#include "Vanilla/Display.h"
#include "Vanilla/Xcb/XcbRasterDrawContext.h"

VANILLA_NS_BEGIN

DrawContext::ScopedFrame::ScopedFrame(Handle<DrawContext> ctx, const SkRect& region)
    : fContext(std::move(ctx)),
      fSurface(nullptr)
{
    fSurface = fContext->beginFrame(region);
}

DrawContext::ScopedFrame::~ScopedFrame()
{
    if (fSurface)
        fContext->endFrame();
}

Handle<DrawContext> DrawContext::MakeRaster(Handle<Window> window)
{
    switch (window->getDisplay()->backend())
    {
    case DisplayBackend::kDisplay_Xcb:
        return std::make_shared<XcbRasterDrawContext>(std::move(window));
    }
}

DrawContext::DrawContext(Handle<Window> window, RasterizerType type)
    : fType(type),
      fWindow(std::move(window)),
      fInFrame(false),
      fRegion(SkRect::MakeEmpty()),
      fLastConfigure(SkRect::MakeWH(fWindow->width(), fWindow->height()))
{
    fWindow->signalConfigure().connect(sigc::mem_fun(*this, &DrawContext::onWindowConfigure));
    va_slot_connect(Window, Configure, fWindow, sigc::mem_fun(*this, &DrawContext::onWindowConfigure));
}

DrawContext::~DrawContext()
{
    va_slot_disconnect(Window, Configure);
}

void DrawContext::onWindowConfigure(const Handle<Window>& window, const SkRect& rect)
{
    if (rect.width() != fLastConfigure.width() ||
        rect.height() != fLastConfigure.height())
    {
        this->onResize(rect.width(), rect.height());
    }
    fLastConfigure = rect;
}

sk_sp<SkSurface> DrawContext::beginFrame(const SkRect& region)
{
    assert(!fInFrame);
    fRegion = region;
    fInFrame = true;
    sk_sp<SkSurface> ret = this->onBeginFrame(region);
    if (ret == nullptr)
        fInFrame = false;
    return ret;
}

void DrawContext::endFrame()
{
    assert(fInFrame);
    this->onEndFrame(fRegion);
    fRegion = SkRect::MakeEmpty();
    fInFrame = false;
}

VANILLA_NS_END
