#include <iostream>

#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Core/EventSource.h"

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderHost.h"
#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHostCreator.h"

#include "general_tests.h"
namespace cocoa::test {

using namespace cobalt;

void vanilla_render_test()
{
    using namespace cobalt;
    using namespace std::string_literals;

    GlobalScope::Ref().Initialize();

    auto creator = GlobalScope::Ref().GetRenderHost()->GetRenderHostCreator();

    creator->Connect(COBALT_SI_RENDERHOSTCREATOR_CREATED, [](RenderHostSlotCallbackInfo& info) {
        fmt::print("Signal slot called\n");
    });

    creator->Invoke(COBALT_OP_RENDERHOSTCREATOR_CREATE_DISPLAY, [](RenderHostCallbackInfo& info) {
        if (info.GetReturnStatus() == RenderClientCallInfo::Status::kCaught)
        {
            fmt::print("Caught: {}\n", info.GetCaughtException().what());
        }
        fmt::print("RenderClient reports object created\n");
        GlobalScope::Ref().Dispose();
    }, "wayland-expire"s);

    EventLoop::Ref().run();
}

}
