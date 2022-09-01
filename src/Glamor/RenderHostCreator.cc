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

#include "Glamor/RenderHostCreator.h"
#include "Glamor/Display.h"
#include "Glamor/Blender.h"
#include "Glamor/Surface.h"
GLAMOR_NAMESPACE_BEGIN

GLAMOR_TRAMPOLINE_IMPL(RenderHostCreator, CreateDisplay)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->As<RenderHostCreator>();

    auto& ret = info.SetReturnValue(this_->CreateDisplay(info.Get<std::string>(0)));
    info.SetReturnStatus(ret ? RenderClientCallInfo::Status::kOpSuccess
                             : RenderClientCallInfo::Status::kOpFailed);
}
RenderHostCreator::RenderHostCreator()
    : RenderClientObject(RealType::kRenderHostCreator)
{
    SetMethodTrampoline(GLOP_RENDERHOSTCREATOR_CREATE_DISPLAY,
                        RenderHostCreator_CreateDisplay_Trampoline);
}

Shared<RenderClientObject> RenderHostCreator::CreateDisplay(const std::string& name)
{
    auto result = Display::Connect(GlobalScope::Ref().GetRenderClient()->GetEventLoop(), name);

    RenderClientEmitterInfo emitterInfo;
    emitterInfo.PushBack(result->Self());
    Emit(GLSI_RENDERHOSTCREATOR_CREATED, std::move(emitterInfo));

    return result;
}

GLAMOR_NAMESPACE_END
