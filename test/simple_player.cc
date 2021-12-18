#include <vector>
#include <iostream>
#include <memory>

#include "fmt/format.h"

#include "include/effects/SkImageFilters.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkFontMgr.h"

#include "Core/Journal.h"
#include "Core/Properties.h"
#include "Core/EventSource.h"
#include "Core/EventLoop.h"
#include "Core/Data.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkImage.h"
#include "Vanilla/Context.h"
#include "Vanilla/Display.h"
#include "Vanilla/Window.h"
#include "Vanilla/DrawContext.h"
#include "Vanilla/RenderKit/ContentAggregator.h"
#include "Vanilla/RenderKit/PictureLayer.h"
#include "Vanilla/RenderKit/ImageLayer.h"
#include "Vanilla/RenderKit/LayerFactories.h"
#include "Vanilla/RenderKit/Renderer.h"

using namespace cocoa;
using namespace vanilla;

class UpdateTimer : public TimerSource
{
public:
    UpdateTimer(EventLoop *loop, Handle<Window> window)
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
    Handle<Window>    fWindow;
};

#if 0
void rotateBackgroundLayers(const std::vector<uint32_t>& ids, Renderer& renderer)
{
    static uint32_t p = 0;
    for (uint32_t i = 0; i < ids.size(); i++)
    {
        renderer.cmdOperateLayer([i](const Handle<Layer>& layer) {
            layer->getProperties()->setMatrix(SkM44::Rotate({1, 1, 1}, M_PI / 180.0f * p));
            layer->getProperties()->setMatrixAA(true);
            return true;
        }, ids[i]);
    }
    p = (p + 5) % 360;
}
#else
void rotateBackgroundLayers(const std::vector<uint32_t>& ids, Renderer& renderer)
{
    static uint32_t p = 0;
    std::vector<Renderer::Command::LayerOperationGroup> ops;
    for (uint32_t i = 0; i < ids.size(); i++)
    {
        ops.push_back({[](const Handle<Layer>& layer) {
            layer->getProperties()->setMatrix(SkM44::Rotate({1, 1, 1}, M_PI / 180.0f * p));
            layer->getProperties()->setMatrixAA(true);
            return true;
        }, ids[i]});
    }
    renderer.cmdOperateLayersConcurrently(ops).wait();
    p = (p + 5) % 360;
}
#endif

int main(int argc, const char **argv)
{
    Journal::New(LOG_LEVEL_DEBUG, Journal::OutputDevice::kStandardOut, true);
    EventLoop::New();
    ScopeEpilogue epilogue([]() -> void {
        EventLoop::Delete();
        Journal::Delete();
    });

    auto context = Context::Make(EventLoop::Instance(), Context::Backend::kXcb);
    context->connectTo(nullptr, 1);
    auto display = context->display(1);
    auto window = display->createWindow({1280, 720}, {0, 0});
    auto dc = DrawContext::MakeVulkan(window);
    Renderer renderer(dc);

    std::vector<uint32_t> bgLayerIds;
    for (uint32_t i = 0; i < 100; i++)
    {
        uint32_t layer0Id = renderer.pushLayer(ImageLayerFactory(ImageAdaptationMethod::kRepeatXY,
                                                                 false,
                                                                 i * 10, i * 10, 800, 600));

        renderer.cmdOperateLayer([](const Handle<Layer>& layer) {
            auto imageLayer = ImageLayer::Cast(layer);
            imageLayer->upload(Data::MakeFromFile("/home/sora/Pictures/Library/ACG/org.faceicon.anime.kafuu-chino.jpeg",
                                                  {vfs::OpenFlags::kReadonly}));
            // imageLayer->getProperties()->setImageFilter(SkImageFilters::Blur(2.1, 2.1, nullptr));
            return true;
        }, layer0Id);
        bgLayerIds.push_back(layer0Id);
    }

    uint32_t layer1Id = renderer.pushLayer(PictureLayerFactory(false,
                                                               100, 100, 400, 300));

    renderer.cmdOperateLayer([](const Handle<Layer>& layer) {
        PictureLayer::Cast(layer)->requestResources();
        return true;
    }, layer1Id);

    sk_sp<SkFontMgr> mgr(SkFontMgr::RefDefault());
    SkTypeface *typeface = mgr->matchFamilyStyle("Calibri", SkFontStyle());

    SkFont font((sk_sp<SkTypeface>(typeface)), 17);

    auto layer = PictureLayer::Cast(renderer.getAggregator()->getLayerById(layer1Id));

    window->signalRepaint().connect([&renderer, &font, &layer, &bgLayerIds](const Handle<Window>& win, const SkRect& region) -> void {
        float fps = renderer.getAggregator()->getFps();
        layer->paint([&font, fps](SkCanvas *canvas) -> void {
            canvas->clear(SK_ColorTRANSPARENT);
            SkPaint paint;
            paint.setAntiAlias(true);
            paint.setColor(SK_ColorBLACK);
            std::string str = fmt::format("FPS meter: {}", fps);
            canvas->drawString(str.c_str(), 20, 20, font, paint);
        });
        rotateBackgroundLayers(bgLayerIds, renderer);
        renderer.cmdActivatePictureLayer(layer->getLayerId());
        renderer.cmdPresent(region);
    });

    UpdateTimer timer(EventLoop::Instance(), window);
    window->signalClose().connect([&renderer, &display, &timer](const Handle<Window>& win) -> void {
        timer.stopTimer();
        renderer.dispose();
        win->close();
        display->dispose();
    });

    window->show();

    EventLoop::Ref().run();

    return 0;
}
