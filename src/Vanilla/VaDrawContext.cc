#include <cassert>

#include "Vanilla/VaDrawContext.h"
#include "Vanilla/VaWindow.h"
#include "Vanilla/VaDisplay.h"
#include "Vanilla/Xcb/VaXcbRasterDrawContext.h"

VANILLA_NS_BEGIN

VaDrawContext::ScopedFrame::ScopedFrame(Handle<VaDrawContext> ctx, const SkRect& region)
    : fContext(std::move(ctx)),
      fSurface(nullptr)
{
    fSurface = fContext->beginFrame(region);
}

VaDrawContext::ScopedFrame::~ScopedFrame()
{
    if (fSurface)
        fContext->endFrame();
}

Handle<VaDrawContext> VaDrawContext::MakeRaster(Handle<VaWindow> window)
{
    switch (window->getDisplay()->backend())
    {
    case DisplayBackend::kDisplay_Xcb:
        return std::make_shared<VaXcbRasterDrawContext>(std::move(window));
    }
}

VaDrawContext::VaDrawContext(Handle<VaWindow> window, RasterizerType type)
    : fType(type),
      fWindow(std::move(window)),
      fInFrame(false),
      fRegion(SkRect::MakeEmpty()),
      fLastConfigure(SkRect::MakeWH(fWindow->width(), fWindow->height()))
{
    fWindow->signalConfigure().connect(sigc::mem_fun(this, &VaDrawContext::onWindowConfigure));
    va_slot_connect(VaWindow, Configure, fWindow, sigc::mem_fun(*this, &VaDrawContext::onWindowConfigure));
}

VaDrawContext::~VaDrawContext()
{
    va_slot_disconnect(VaWindow, Configure);
}

void VaDrawContext::onWindowConfigure(const Handle<VaWindow>& window, const SkRect& rect)
{
    if (rect.width() != fLastConfigure.width() ||
        rect.height() != fLastConfigure.height())
    {
        this->onResize(rect.width(), rect.height());
    }
    fLastConfigure = rect;
}

sk_sp<SkSurface> VaDrawContext::beginFrame(const SkRect& region)
{
    assert(!fInFrame);
    fRegion = region;
    fInFrame = true;
    sk_sp<SkSurface> ret = this->onBeginFrame(region);
    if (ret == nullptr)
        fInFrame = false;
    return ret;
}

void VaDrawContext::endFrame()
{
    assert(fInFrame);
    this->onEndFrame(fRegion);
    fRegion = SkRect::MakeEmpty();
    fInFrame = false;
}

VANILLA_NS_END
