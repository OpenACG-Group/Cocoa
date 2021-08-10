#include <iostream>

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"

#include "Core/EventLoop.h"
#include "Core/EventSource.h"
#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
#include "Vanilla/Window.h"
#include "Vanilla/DrawContext.h"

#include <gperftools/profiler.h>
#include "general_tests.h"
namespace cocoa::test {

using namespace vanilla;

void draw(SkCanvas* canvas)
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

    KeepInLoop timerDispatch() override
    {
        fWindow->update();
        return KeepInLoop::kYes;
    }

private:
    Handle<VaWindow>    fWindow;
};

void vanilla_render_test()
{
    // ProfilerStart("profiler.pprof");

    auto ctx = Context::MakeX11(EventLoop::Instance());
    ctx->open(nullptr, Context::kDisplay_Default);

    auto window = VaWindow::Make(ctx->display(Context::kDisplay_Default),
                                          VaVec2f(400, 300),
                                          VaVec2f(0, 0));
    auto drawContext = VaDrawContext::MakeRaster(window);

    window->show();
    window->setTitle("Vanilla");
    window->setIconFile("/home/sora/Project/C++/Cocoa/res/koinu.png");

    window->signalRepaint().connect([&drawContext](const Handle<VaWindow>& win, const SkRect& rect) -> void {
        sk_sp<SkSurface> surface = drawContext->beginFrame(rect);
        if (surface == nullptr)
            return;
        draw(surface->getCanvas());
        drawContext->endFrame();
    });

    UpdateTimer timer(EventLoop::Instance(), window);
    timer.stopTimer();
    window->signalClose().connect([&timer](const Handle<VaWindow>& win) -> void {
        win->close();
        win->getDisplay()->dispose();
        timer.stopTimer();
    });

    EventLoop::Ref().run();

    // ProfilerStop();
}

}
