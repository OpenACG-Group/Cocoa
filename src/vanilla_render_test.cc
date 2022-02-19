#include <iostream>

#include "Core/Journal.h"
#include "Core/EventLoop.h"
#include "Core/EventSource.h"

#include "Cobalt/Cobalt.h"
#include "Cobalt/RenderClient.h"
#include "Cobalt/RenderHost.h"

#include "Cobalt/RenderClientObject.h"
#include "Cobalt/RenderHostCreator.h"
#include "Cobalt/Display.h"

#include "general_tests.h"
namespace cocoa::test {

using namespace cobalt;

void vanilla_render_test()
{
    using namespace cobalt;
    using namespace std::string_literals;

    GlobalScope::Ref().Initialize();

    EventLoop::Ref().run();
}

}
