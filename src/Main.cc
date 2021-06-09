#include <iostream>
#include <Poco/Stopwatch.h>
#include <gperftools/profiler.h>

#include "Core/PosixSignalCatcher.h"
#include "Core/Utils.h"
#include "Core/Journal.h"
#include "Core/Exception.h"
#include "Core/Configurator.h"
#include "Core/EventLoop.h"

#include "Scripter/Runtime.h"
#include "Vanilla/Context.h"
#include "Vanilla/VaDisplay.h"
#include "Vanilla/VaWindow.h"

namespace cocoa
{

#if 0
class MyWindow : public NaGui::Window
{
public:
    ~MyWindow() override = default;

    void onRepaint(NaGui::Draw& draw) override
    {
        draw.test();
    }
};

void RendererEntry()
{
    ciallo::XcbConnection::New();
    NaGui::Context::New();

    auto drawable = new ciallo::XcbWindow(ciallo::XcbConnection::Ref().screen(),
                                          SkIRect::MakeXYWH(0, 0, 400, 300));
    drawable->setResizable(true);
    drawable->setTitle("NativeGui Test");
    NaGui::BindDrawableWindow(drawable, NaGui::Window::Make<MyWindow>());

    int32_t events;
    while ((events = EventDispatcher::Ref().wait()))
        EventDispatcher::Ref().handleEvents(events);
}

#endif

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

    EventLoop::New();
    PosixSignalCatcher::New();

    return Configurator::State::kSuccessful;
}

void Finalize()
{
    scripter::Dispose();
    PosixSignalCatcher::Delete();
    EventLoop::Delete();
    Journal::Delete();
    PropertyTree::Delete();
}

void Run()
{
    log_write(LOG_DEBUG) << "Content of property tree:" << log_endl;
    utils::DumpPropertyTree(PropertyTree::Ref()("/"), [](const std::string& str) -> void {
        log_write(LOG_DEBUG) << str << log_endl;
    });

    auto ctx = vanilla::Context::MakeX11(EventLoop::Instance(), nullptr);
    auto window = vanilla::VaWindow::Make(ctx->display(),
                                          vanilla::VaVec2f(400, 300),
                                          vanilla::VaVec2f(0, 0));
    window->show();
    window->setTitle("Vanilla");
    window->setIconFile("/home/sora/Project/C++/Cocoa/res/koinu.png");

    window->signalClose().connect([](const vanilla::Handle<vanilla::VaWindow>& win) -> void {
        win->close();
        win->getDisplay()->dispose();
    });
    // window->update();

    EventLoop::Ref().run();

#if 0
    scripter::Initialize();
    auto runtime = scripter::Runtime::MakeFromSnapshot("/home/sora/Project/C++/Cocoa/cmake-build-debug/snapshot_blob.bin",
                                                       "/home/sora/Project/C++/Cocoa/cmake-build-debug/icudtl.dat");
    v8::Isolate::Scope isolateScope(runtime->isolate());
    v8::HandleScope handleScope(runtime->isolate());
    v8::Context::Scope contextScope(runtime->context());
    const char *script = R"(
vmInvokeOp("op_print_async", {str: "Hello, World!"})
    .then(() => {
        vmInvokeOp("op_print", {str: "Asynchronous succeed"});
    });
)";
    runtime->execute(script);
    sleep(1);
    runtime->runPromisesCheckpoint();
#endif

#if 0
    komorebi::InitializeShaderCompiler();

    auto program = komorebi::ShaderProgram::Make();
    komorebi::ShaderModule::MakeFromSource("TestShader",
                                           program,
                                           "test source")->moveToProgram();

    auto sym = program->lookupSymbol("main");
    if (!sym)
    {
        std::cerr << "Failed to compile" << std::endl;
        return;
    }
    auto fn = reinterpret_cast<int(*)()>(sym->getAddress());

    std::cout << "Result: " << fn() << std::endl;
#endif

#if 0
    komorebi::KRSLParserDriver drv;
    drv.parse(std::cin, std::cout);
#endif
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
