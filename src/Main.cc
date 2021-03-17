#define TEST_CIALLO         1

#include <xcb/xcb.h>

#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Configurator.h"
#include "Core/EventDispatcher.h"

#include "crpkg/Deserializer.h"

#include "Ciallo/XcbConnection.h"
#include "Ciallo/XcbEventQueue.h"
#include "Ciallo/XcbScreen.h"
#include "Ciallo/XcbWindow.h"
#include "Ciallo/Skia2d/GrContext.h"
#include "Ciallo/Cairo2d/CrSurface.h"
#include "Ciallo/Cairo2d/CrCanvas.h"

#if TEST_CIALLO
#include "include/core/SkPicture.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#endif

#include <memory>
#include <iostream>
#include <Poco/Stopwatch.h>
#include <gperftools/profiler.h>

namespace cocoa
{

using namespace ciallo;
class Renderer : public DrawableListener
{
public:
    explicit Renderer(Drawable *drawable, XcbConnection *conn)
            : fCtx(drawable, GrContextOptions::MakeFromPropertyTree()),
              fConn(conn)
    {
        layer = fCtx.comp()->overlay(800, 600, 0, 0, 0);
    }

    ~Renderer() override = default;

    void onRender() override
    {
        SkPictureRecorder rec;
        SkCanvas *canvas = rec.beginRecording(layer->width(), layer->height());

        auto mgr = SkFontMgr::RefDefault();
        auto *set = mgr->matchFamily("Consolas");
        sk_sp<SkTypeface> typeface = nullptr;
        for (int32_t i = 0; i < set->count(); i++)
        {
            SkFontStyle style;
            SkString styleName;
            set->getStyle(i, &style, &styleName);
            if (styleName.equals("Italic"))
                typeface.reset(set->createTypeface(i));
        }
        set->unref();

        canvas->clear(SK_ColorWHITE);
        SkFont font(typeface);
        SkPaint paint;
        font.setSize(30);
        canvas->drawString("Hello, World", 20, 100, font, paint);

        layer->paint(rec.finishRecordingAsPicture(), 0, 0);
        layer->update();
        fCtx.comp()->present();
    }

    bool onClose() override
    {
        fConn->disconnect();
        return true;
    }

private:
    GrContext      fCtx;
    std::shared_ptr<GrBaseRenderLayer> layer;
    XcbConnection *fConn;
};

class CrRenderer : public DrawableListener
{
public:
    CrRenderer(Drawable *drawable, XcbConnection *conn)
        : fSurface(CrSurface::MakeFromDrawable(drawable)),
          fCanvas(fSurface),
          fConn(conn)
    {
        fCanvas.setSource(CrSurface::MakeImage("src.png"), 0, 0);
    }

    ~CrRenderer() override = default;

    void onRender() override
    {
        /*
        fCanvas.selectFontFace("Consolas", CAIRO_FONT_SLANT_ITALIC, CAIRO_FONT_WEIGHT_NORMAL);
        fCanvas.setFontSize(30);
        fCanvas.moveTo(20, 100);
        fCanvas.drawText("Hello, World");
        */
        // 2c8fb2
        fCanvas.setAntialias(CAIRO_ANTIALIAS_BEST);
        fCanvas.drawArc(300, 300, 250, 0, M_PI * 2);
        fCanvas.drawFill();
        // fSurface->flush();
    }

    bool onClose() override
    {
        fConn->disconnect();
        return true;
    }

private:
    std::shared_ptr<CrSurface> fSurface;
    CrCanvas fCanvas;
    XcbConnection *fConn;
};

void xcbRender()
{
    XcbConnection connection;
    XcbWindow window(connection.screen(), SkIRect::MakeXYWH(0, 0, 800, 600));

    window.setTitle("Hello, World");
    window.setResizable(true);

    auto renderer = std::make_shared<CrRenderer>(&window, &connection);
    window.setListener(renderer);

    int32_t events;
    while ((events = EventDispatcher::Instance()->wait()))
        EventDispatcher::Instance()->handleEvents(events);
}

Configurator::State Initialize(int argc, char const **argv)
{
    PropertyTree::New();
    PropertyTree *prop = PropertyTree::Instance();

    Configurator conf;
    Configurator::State state = conf.parse(argc, argv);
    if (state == Configurator::State::kShouldExitNormally ||
        state == Configurator::State::kError)
        return state;

    std::string level = prop->resolve("/runtime/journal/level")
                            ->cast<PropertyTreeDataNode>()->extract<std::string>();

    int filter;
    if (level == "debug")
        filter = LogLevel::LOG_LEVEL_DEBUG;
    else if (level == "normal")
        filter = LogLevel::LOG_LEVEL_NORMAL;
    else if (level == "quiet")
        filter = LogLevel::LOG_LEVEL_QUIET;
    else if (level == "silent")
        filter = LogLevel::LOG_LEVEL_SILENT;
    else if (level == "disabled")
        filter = LogLevel::LOG_LEVEL_DISABLED;
    else
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Unknown log level: ")
                .append(level)
                .make<RuntimeException>();
    }

    bool rainbow = prop->resolve("/runtime/journal/textShader")
            ->cast<PropertyTreeDataNode>()->value().extract<bool>();

    std::string redirect = prop->resolve("/runtime/journal/stdout")
                               ->cast<PropertyTreeDataNode>()->value().extract<std::string>();
    if (redirect == "<stdout>")
        Journal::New(STDOUT_FILENO, filter, rainbow);
    else if (redirect == "<stderr>")
        Journal::New(STDERR_FILENO, filter, rainbow);
    else
        Journal::New(redirect.c_str(), filter, rainbow);

    EventDispatcher::New();

    return Configurator::State::kSuccessful;
}

void Finalize()
{
    EventDispatcher::Delete();
    Journal::Delete();
    PropertyTree::Delete();
}

void Run()
{
    log_write(LOG_DEBUG) << "Content of property tree:" << log_endl;
    utils::DumpPropertyTree(PropertyTree::Ref()("/"), [](const std::string& str) -> void {
        log_write(LOG_DEBUG) << str << log_endl;
    });

    // xcbRender();
    crpkg::Deserializer deserializer("Test.crpkg.blob");
    auto *node = PropertyTreeNode::NewDirNode(PropertyTree::Ref()("/"),
                                              "packages")->cast<PropertyTreeDirNode>();
    deserializer.extractTo(node);

    utils::DumpPropertyTree(PropertyTree::Ref()("/"), [](const std::string& str) -> void {
        log_write(LOG_DEBUG) << str << log_endl;
    });
}

int Main(int argc, char const **argv)
{
    BeforeLeaveScope beforeLeaveScope([]() -> void { Finalize(); });
    try
    {
        switch (Initialize(argc, argv))
        {
        case Configurator::State::kError:
            return 1;
        case Configurator::State::kShouldExitNormally:
            return 0;
        case Configurator::State::kSuccessful:
            break;
        }

        Run();
    }
    catch (const RuntimeException& e)
    {
        if (Journal::Instance())
        {
            bool color = PropertyTree::Instance()->resolve("/runtime/journal/exceptionTextShader")
                            ->cast<PropertyTreeDataNode>()->extract<bool>();
            utils::DumpRuntimeException(e, color, [](const std::string& str) -> void {
                log_write(LOG_EXCEPTION) << str << log_endl;
            });
        }
        else
        {
            utils::DumpRuntimeException(e, false, [](const std::string& str) -> void {
                std::cerr << str << std::endl;
            });
        }
    }

    return 0;
}

} // namespace cocoa

int main(int argc, char const *argv[])
{
    return cocoa::Main(argc, argv);
}
