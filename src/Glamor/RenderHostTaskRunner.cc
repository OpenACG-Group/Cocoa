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
    SetMethodTrampoline(CROP_TASKRUNNER_RUN, RenderHostTaskRunner_Run_Trampoline);
}

void RenderHostTaskRunner::Run(const Task& task)
{
    task();
}

GLAMOR_NAMESPACE_END
