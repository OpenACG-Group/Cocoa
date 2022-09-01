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

#include "Glamor/RenderHostTaskRunner.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

GLAMOR_TRAMPOLINE_IMPL(RenderHostTaskRunner, Run)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->As<RenderHostTaskRunner>();
    this_->Run(info.Get<RenderHostTaskRunner::Task>(0));
}

} // namespace anonymous

RenderHostTaskRunner::RenderHostTaskRunner()
    : RenderClientObject(RealType::kRenderHostTaskRunner)
{
    SetMethodTrampoline(GLOP_TASKRUNNER_RUN, RenderHostTaskRunner_Run_Trampoline);
}

void RenderHostTaskRunner::Run(const Task& task)
{
    task();
}

GLAMOR_NAMESPACE_END
