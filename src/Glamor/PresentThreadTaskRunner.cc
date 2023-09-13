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

#include "Glamor/PresentThreadTaskRunner.h"
GLAMOR_NAMESPACE_BEGIN

namespace {

GLAMOR_TRAMPOLINE_IMPL(PresentThreadTaskRunner, Run)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto this_ = info.GetThis()->As<PresentThreadTaskRunner>();
    info.SetReturnValueAny(this_->Run(info.GetConst<PresentThreadTaskRunner::Task>(0)));
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

} // namespace anonymous

PresentThreadTaskRunner::PresentThreadTaskRunner()
    : PresentRemoteHandle(RealType::kTaskRunner)
{
    SetMethodTrampoline(GLOP_TASKRUNNER_RUN, PresentThreadTaskRunner_Run_Trampoline);
}

std::any PresentThreadTaskRunner::Run(const Task& task)
{
    return task();
}

GLAMOR_NAMESPACE_END
