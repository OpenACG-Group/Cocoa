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
