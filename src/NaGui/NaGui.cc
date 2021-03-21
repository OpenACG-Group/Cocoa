#include <memory>

#include "Core/Journal.h"
#include "Core/BaseEventProcessor.h"
#include "Core/EventDispatcher.h"
#include "Ciallo/DrawableListener.h"
#include "NaGui/NaGui.h"
#include "NaGui/NaSharedState.h"
NAGUI_NS_BEGIN

class NaWindowCollector : public BaseEventProcessor
{
public:
    ~NaWindowCollector() override = default;
    void processEvent() override;
};

void NaWindowCollector::processEvent()
{
    auto& queue = NaContext::Ref().fClosedwindows;
    if (queue.empty())
    {
        log_write(LOG_ERROR) << "NaWindowCollector: We're notified to collect dead windows"
                             << " but there's noting in the queue." << log_endl;
        return;
    }

    auto *ptr = queue.back();
    queue.pop();
    NaContext::Ref().removeDrawable(ptr);
}

// Relationship between these classes:
// (A -> B means you can get B by A)
//
// Drawable -> NaWindowListener -> NaWindow
//    ^                              |
//    `------------------------------'
class NaWindowListener : public ciallo::DrawableListener
{
public:
    explicit NaWindowListener(NaWindow::Ptr window, NaSharedState *state);
    ~NaWindowListener() override;

    inline NaSharedState *sharedState()
    {
        return fState;
    }

    bool onClose() override;
    void onRender(ciallo::DrEventPtr<ciallo::DrRepaintEvent> event) override;
    void onButtonPress(ciallo::DrEventPtr<ciallo::DrButtonPressEvent> event) override;
    void onButtonRelease(ciallo::DrEventPtr<ciallo::DrButtonReleaseEvent> event) override;
    void onMotion(ciallo::DrEventPtr<ciallo::DrMotionEvent> event) override;

private:
    NaWindow::Ptr       fWindow;
    NaSharedState      *fState;
};

NaWindowListener::NaWindowListener(NaWindow::Ptr window, NaSharedState *state)
    : fWindow(std::move(window)),
      fState(state)
{
}

NaWindowListener::~NaWindowListener()
{
    delete fState;
}

bool NaWindowListener::onClose()
{
    NaContext::Ref().collectClosedWindowLater(fWindow->drawable());
    return false;
}

void NaWindowListener::onRender(ciallo::DrEventPtr<ciallo::DrRepaintEvent> event)
{
    fWindow->paintEvent();
}

void NaWindowListener::onButtonPress(ciallo::DrEventPtr<ciallo::DrButtonPressEvent> event)
{
    switch (event->button())
    {
    case ciallo::DrButton::kLeftButton:
        fState->mouseLeftButton.set(true);
        break;

    case ciallo::DrButton::kRightButton:
        fState->mouseRightButton.set(true);
        break;

    case ciallo::DrButton::kMiddleButton:
        fState->mouseMiddleButton.set(true);
        break;
    }
}

void NaWindowListener::onButtonRelease(ciallo::DrEventPtr<ciallo::DrButtonReleaseEvent> event)
{
    switch (event->button())
    {
    case ciallo::DrButton::kLeftButton:
        fState->mouseLeftButton.set(false);
        break;

    case ciallo::DrButton::kRightButton:
        fState->mouseRightButton.set(false);
        break;

    case ciallo::DrButton::kMiddleButton:
        fState->mouseMiddleButton.set(false);
        break;
    }
}

void NaWindowListener::onMotion(ciallo::DrEventPtr<ciallo::DrMotionEvent> event)
{
    fState->mouseX.set(event->x());
    fState->mouseY.set(event->y());
}

// ------------------------------------------------------------------------------------

NaContext::NaContext()
    : fCollector(new NaWindowCollector())
{
}

NaContext::~NaContext()
{
    for (ciallo::Drawable *ptr : fDrawables)
        delete ptr;
    delete fCollector;
}

void NaContext::addDrawable(ciallo::Drawable *pDrawable)
{
    if (!pDrawable)
        return;
    fDrawables.push_back(pDrawable);
}

void NaContext::removeDrawable(ciallo::Drawable *pDrawable)
{
    if (!pDrawable)
        return;

    bool found = false;
    fDrawables.remove_if([pDrawable, &found](ciallo::Drawable *ptr) -> bool {
        if (ptr == pDrawable)
        {
            found = true;
            return true;
        }
        return false;
    });

    if (found)
    {
        /**
         * Our listener owns some resources of Drawable,
         * resetting the listener to a fake one will destruct
         * the real listener (NaWindowListener), then we can destruct
         * Drawable safely.
         */
        pDrawable->setListener(ciallo::DrawableListener::Make<ciallo::DrawableListener>());
        delete pDrawable;
    }
}

void NaContext::collectClosedWindowLater(ciallo::Drawable *pDrawable)
{
    fClosedwindows.push(pDrawable);
    EventDispatcher::Ref().wakeup(fCollector);
}

// -------------------------------------------------------------------------------------

void NaWindow::paint()
{
    if (fDrawable)
        fDrawable->repaint();
}

void NaWindow::_privSetSurface(ciallo::CrSurface::Ptr surface)
{
    fSurface = std::move(surface);
    fCanvas = std::make_unique<ciallo::CrCanvas>(fSurface);
}

void NaWindow::_privSetDrawable(ciallo::Drawable *pDrawable)
{
    fDrawable = pDrawable;
}

void NaWindow::_privSetSharedState(NaSharedState *state)
{
    fState = state;
}

void NaWindow::paintEvent()
{
    fCanvas->setSource(0.94, 0.94, 0.94);
    fCanvas->drawPaint();

    applyStateUpdates();
    fCanvas->save();
    this->onRepaint();
    fCanvas->restore();

    fSurface->flush();
}

static cairo_font_slant_t toCairoFontSlant(FontSlant slant)
{
    switch (slant)
    {
    case FontSlant::kSlant_Normal:
        return CAIRO_FONT_SLANT_NORMAL;
    case FontSlant::kSlant_Italic:
        return CAIRO_FONT_SLANT_ITALIC;
    case FontSlant::kSlant_Oblique:
        return CAIRO_FONT_SLANT_OBLIQUE;
    }
}

static cairo_font_weight_t toCairoFontWeight(FontWeight weight)
{
    switch (weight)
    {
    case FontWeight::kWeight_Bold:
        return CAIRO_FONT_WEIGHT_BOLD;
    case FontWeight::kWeight_Normal:
        return CAIRO_FONT_WEIGHT_NORMAL;
    }
}

static cairo_antialias_t toCairoAntialias(bool antls)
{
    if (antls)
        return CAIRO_ANTIALIAS_SUBPIXEL;
    return CAIRO_ANTIALIAS_NONE;
}

void NaWindow::applyStateUpdates()
{
    if (fState->fontAntialias.changed())
    {
        auto *options = cairo_font_options_create();
        cairo_font_options_set_antialias(options,
                                         toCairoAntialias(fState->fontAntialias.value()));
        fCanvas->setFontOptions(options);
        cairo_font_options_destroy(options);
    }

    if (fState->fontFamily.changed() ||
        fState->fontSlant.changed() ||
        fState->fontWeight.changed())
    {
        fCanvas->selectFontFace(fState->fontFamily.value(),
                                toCairoFontSlant(fState->fontSlant.value()),
                                toCairoFontWeight(fState->fontWeight.value()));
    }

    if (fState->fontSize.changed())
        fCanvas->setFontSize(fState->fontSize.value());

    if (fState->windowTitle.changed())
    {
        fDrawable->setTitle(fState->windowTitle.value());
    }

    if (fState->windowResizable.changed())
    {
        fDrawable->setResizable(fState->windowResizable.value());
    }
}

void NaWindow::SetFontAntialias(bool antialias)
{
    fState->fontAntialias.set(antialias);
}

void NaWindow::SetFontFamily(const std::string& family)
{
    fState->fontFamily.set(family);
}

void NaWindow::SetFontSize(double size)
{
    fState->fontSize.set(size);
}

void NaWindow::SetFontStyle(FontSlant slant, FontWeight weight)
{
    fState->fontSlant.set(slant);
    fState->fontWeight.set(weight);
}

void NaWindow::SetWindowTitle(const std::string& title)
{
    fState->windowTitle.set(title);
}

void NaWindow::SetWindowResizable(bool resizable)
{
    fState->windowResizable.set(resizable);
}

// -------------------------------------------------------------------------------------

void BindDrawableWindow(ciallo::Drawable *pDrawable,
                        NaWindow::Ptr window)
{
    auto *state = new NaSharedState();
    window->_privSetDrawable(pDrawable);
    window->_privSetSurface(ciallo::CrSurface::MakeFromDrawable(pDrawable));
    window->_privSetSharedState(state);

    auto listener = std::make_shared<NaWindowListener>(std::move(window), state);
    pDrawable->setListener(listener);
    NaContext::Ref().addDrawable(pDrawable);
}

NAGUI_NS_END
