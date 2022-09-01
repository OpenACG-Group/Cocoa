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

#include <iostream>
#include <utility>
#include <random>
#include <chrono>

#include "general_tests.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"

#include "Core/EventSource.h"
#include "Core/RandomDevice.h"

#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
#include "Vanilla/Window.h"
#include "Vanilla/DrawContext.h"

#include "Vanilla/Shader/ShaderExecutor.h"

using namespace cocoa;
using namespace cocoa::vanilla;
namespace cocoa::test {

class ParticleSimulator
{
public:
    enum {
        kMaxCategory = 3,
        kMaxAge = 650,
        kMaxParticles = 1000
    };

    struct Particle
    {
        int         id = 0;
        VaVec2f     pos{0, 0};
        VaVec2f     velocity{0, 0};
        SkScalar    radius = 6;
        SkColor     color = 0;
        int         age = 0;
        int         category = 3;
    };

    ParticleSimulator() : fEmitterCount(0) {}
    ~ParticleSimulator() = default;

    void draw(SkCanvas *pCanvas, int32_t width, int32_t height)
    {
        pCanvas->clear(SK_ColorBLACK);

        SkPath paths[kMaxCategory];
        for (Particle& p : fParticles)
        {
            float x = width / 2.0f + p.pos.x();
            float y = height / 2.0f - p.pos.y();
            if (x >= width || y >= height || x < 0 || y < 0)
                continue;

            float radius = float(kMaxAge - p.age) / float(kMaxAge) * float(p.radius);
            paths[p.category].addCircle(x, y, radius);
        }

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setAntiAlias(true);
        paint.setBlendMode(SkBlendMode::kPlus);

        constexpr SkColor colors[] = {
                SkColor(0xFFFF7F00),
                SkColor(0xFFFF3F9F),
                SkColor(0xFF7F4FFF)
        };

        for (int i = 0; i < kMaxCategory; i++)
        {
            paint.setColor(colors[i]);
            pCanvas->drawPath(paths[i], paint);
        }
    }

    void evaluate()
    {
        int j = 0;
        for (auto & p : fParticles)
        {
            p.pos = p.pos + p.velocity;
            if (++p.age >= kMaxAge)
                continue;
            fParticles[j++] = p;
        }
        if (j > 0)
            fParticles.resize(j);

        unsigned long long seed;
        __builtin_ia32_rdrand64_step(&seed);

        std::mt19937 e(seed);
        std::uniform_real_distribution<double> rnd(0, 1);

        auto count = int32_t(rnd(e) * static_cast<int>(kMaxParticles));
        for (int32_t i = 0; i < count; i++)
        {
            if (fParticles.size() >= kMaxParticles)
                break;

            VaScalar angle = rnd(e) * M_PI * 2.0;
            VaScalar speed = std::max(rnd(e) * 6.0, 0.6);

            Particle p;
            p.id = fEmitterCount;
            p.pos = {0, 0};
            p.velocity = {speed * std::cos(angle), speed * std::sin(angle)};
            p.age = int(std::min(rnd(e), 0.5) * static_cast<int>(kMaxAge));
            p.category = int(rnd(e) * double(kMaxCategory - 1));
            fParticles.push_back(p);
            fEmitterCount++;
        }
    }

private:
    std::vector<Particle>   fParticles;
    int                     fEmitterCount;
};

class CallbackTimer : public TimerSource
{
public:
    CallbackTimer(EventLoop *loop, std::function<KeepInLoop(void)>  func)
        : TimerSource(loop), fFunc(std::move(func))
    {
        startTimer(1000, 16);
    }
    ~CallbackTimer() override = default;

    KeepInLoop timerDispatch() override
    { return fFunc(); }

private:
    std::function<KeepInLoop(void)> fFunc;
};

void vanilla_xcb_test()
{
    auto executor = ShaderExecutor::Create();
    auto context = Context::Make(EventLoop::Instance(), Context::Backend::kXcb);
    context->connectTo(nullptr, Context::kDefault);

    auto w = VaWindow::Make(context->display(Context::kDefault),
                            {400, 300}, {0, 0});

    w->show();
    w->setTitle("Vanilla");
    w->setIconFile("/home/sora/Project/C++/Cocoa/res/koinu.png");

    auto drawContext = VaDrawContext::MakeVulkan(w);

    ParticleSimulator particle;
    CallbackTimer timer(EventLoop::Instance(), [&w, &particle]() -> KeepInLoop {
        particle.evaluate();
        w->update();
        return KeepInLoop::kYes;
    });

    w->signalRepaint().connect([&drawContext, &particle](const Handle<VaWindow>& win, const SkRect& rect) -> void {
        VaDrawContext::ScopedFrame scope(drawContext, rect);
        if (scope.surface())
        {
            particle.draw(scope.surface()->getCanvas(),
                          scope.surface()->width(),
                          scope.surface()->height());
        }
    });

    w->signalKeyPress().connect([](const Handle<VaWindow>& win, KeySymbol symbol, Bitfield<KeyModifier> mods, Bitfield<KeyLed> leds) -> void {
        std::cout << "KeyPress: " << GetKeySymbolName(symbol) << std::endl;
    });

    w->signalClose().connect([&timer](const Handle<VaWindow>& win) -> void {
        win->close();
        win->getDisplay()->dispose();
        timer.stopTimer();
        std::cout << "window closed, display disposed" << std::endl;
    });

    EventLoop::Ref().run();
}

}
