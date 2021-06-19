#include <iostream>
#include "general_tests.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"

#include "Core/EventSource.h"

#include "Vanilla/Context.h"
#include "Vanilla/VaDisplay.h"
#include "Vanilla/VaWindow.h"
#include "Vanilla/VaDrawContext.h"

using namespace cocoa;
using namespace cocoa::vanilla;
namespace cocoa::test {

void draw(SkCanvas *canvas)
{
    static uint64_t t = 0;
    canvas->drawColor(SK_ColorWHITE);

    SkPaint paint;
    paint.setStyle(SkPaint::kFill_Style);
    paint.setAntiAlias(true);
    paint.setStrokeWidth(4);
    paint.setColor(0xff4285F4);

    SkRect rect = SkRect::MakeXYWH(t % 100, t % 100, 100, 160);
    canvas->drawRect(rect, paint);

    SkRRect oval;
    oval.setOval(rect);
    oval.offset(t % 50, t % 50);
    paint.setColor(0xffDB4437);
    canvas->drawRRect(oval, paint);

    paint.setColor(0xff0F9D58);
    canvas->drawCircle(180, 50, 25, paint);

    rect.offset(80, 50);
    paint.setColor(0xffF4B400);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawRoundRect(rect, 10, 10, paint);

    t++;
}

class UpdateTimer : public TimerSource
{
public:
    UpdateTimer(EventLoop *loop, Handle<VaWindow> window)
            : TimerSource(loop), fWindow(std::move(window))
    {
        startTimer(1000, 16);
    }
    ~UpdateTimer() override = default;

    KeepInLoop dispatch() override
    {
        fWindow->update();
        return KeepInLoop::kYes;
    }

private:
    Handle<VaWindow>    fWindow;
};

void vanilla_xcb_test()
{
    auto context = Context::Make(EventLoop::Instance(), Context::Backend::kBackend_X11);
    context->open(nullptr, Context::kDisplay_Default);

    auto w = VaWindow::Make(context->display(Context::kDisplay_Default),
                            {400, 300}, {0, 0});

    w->show();
    w->setTitle("Vanilla");
    w->setIconFile("/home/sora/Project/C++/Cocoa/res/koinu.png");

    auto drawContext = VaDrawContext::MakeVulkan(w);

    w->signalRepaint().connect([&drawContext](const Handle<VaWindow>& win, const SkRect& rect) -> void {
        VaDrawContext::ScopedFrame scope(drawContext, rect);
        if (scope.surface())
            draw(scope.surface()->getCanvas());
    });

    UpdateTimer timer(EventLoop::Instance(), w);
    w->signalClose().connect([&timer](const Handle<VaWindow>& win) -> void {
        win->close();
        win->getDisplay()->dispose();
        timer.stopTimer();
        std::cout << "window closed, display disposed" << std::endl;
    });

    EventLoop::Ref().run();
}

}
