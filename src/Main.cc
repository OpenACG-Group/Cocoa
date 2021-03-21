#define TEST_CIALLO         1

#include <xcb/xcb.h>

#include "Core/Project.h"
#include "Core/PosixSignalCatcher.h"
#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Configurator.h"
#include "Core/EventDispatcher.h"

#include "NaGui/NaGui.h"
#include "Ciallo/XcbConnection.h"
#include "Ciallo/XcbEventQueue.h"
#include "Ciallo/XcbScreen.h"
#include "Ciallo/XcbWindow.h"

#include <memory>
#include <iostream>
#include <Poco/Stopwatch.h>
#include <gperftools/profiler.h>

namespace cocoa
{

class MyWindow : public NaGui::NaWindow
{
public:
    ~MyWindow() override = default;

    void onRepaint() override
    {
        std::cout << "repaint" << std::endl;
        SetWindowTitle("MyWindow");
        SetWindowResizable(true);

        TextLabel("Hello, World!");
    }
};

void RendererEntry()
{
    ciallo::XcbConnection::New();
    NaGui::NaContext::New();

    NaGui::BindDrawableWindow(
            new ciallo::XcbWindow(ciallo::XcbConnection::Ref().screen(),
                                  SkIRect::MakeXYWH(0, 0, 400, 300)),
            NaGui::NaWindow::Make<MyWindow>());

    int32_t events;
    while ((events = EventDispatcher::Ref().wait()))
        EventDispatcher::Ref().handleEvents(events);
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
    PosixSignalCatcher::New();

    return Configurator::State::kSuccessful;
}

void Finalize()
{
    NaGui::NaContext::Delete();
    ciallo::XcbConnection::Delete();
    PosixSignalCatcher::Delete();
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

    RendererEntry();
    log_write(LOG_INFO) << "Renderer finished" << log_endl;
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
