#include "Core/Errors.h"

#include "Vanilla/DrawContext.h"
#include "Vanilla/Window.h"
#include "Vanilla/Display.h"
#include "Vanilla/Xcb/XcbRasterDrawContext.h"

VANILLA_NS_BEGIN

DrawContext::PresentationScope::PresentationScope(Handle<DrawContext> ctx, const SkRect& region)
    : fContext(std::move(ctx)),
      fSurface(nullptr)
{
    CHECK(fContext);
    fSurface = fContext->beginFrame(region);
}

DrawContext::PresentationScope::~PresentationScope()
{
    if (fSurface)
        fContext->endFrame();
}

DrawContext::DrawScope::DrawScope(Handle<DrawContext> ctx)
    : fContext(std::move(ctx))
{
    CHECK(fContext);
    fContext->lockContext();
}

DrawContext::DrawScope::~DrawScope()
{
    if (fContext)
        fContext->unlockContext();
}

Handle<DrawContext> DrawContext::MakeRaster(Handle<Window> window)
{
    switch (window->getDisplay()->backend())
    {
    case DisplayBackend::kXcb:
        return std::make_shared<XcbRasterDrawContext>(std::move(window));
    case DisplayBackend::kWayland:
        return nullptr;
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

SkColorType DrawContext::getColorType() const
{
    return fWindow->format();
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
    CHECK(!fInFrame);
    fRegion = region;
    fInFrame = true;
    sk_sp<SkSurface> ret = this->onBeginFrame(region);
    if (ret == nullptr)
        fInFrame = false;
    return ret;
}

void DrawContext::endFrame()
{
    CHECK(fInFrame);
    this->onEndFrame(fRegion);
    fRegion = SkRect::MakeEmpty();
    fInFrame = false;
}

void DrawContext::lockContext()
{
    fRasterMutex.lock();
}

void DrawContext::unlockContext()
{
    fRasterMutex.unlock();
}

bool DrawContext::tryLockContext()
{
    return fRasterMutex.try_lock();
}

VANILLA_NS_END
