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

#include "include/utils/SkTraceEventPhase.h"

#include "Core/Utils.h"
#include "Core/TraceEvent.h"
#include "Glamor/SkEventTracerImpl.h"
GLAMOR_NAMESPACE_BEGIN

SkEventTracerImpl::SkEventTracerImpl()
    : trace_started_(false)
{
}

SkEventTracerImpl::~SkEventTracerImpl() = default;

void SkEventTracerImpl::StartTracing(const std::vector<std::string>& enabled_categories)
{
    if (trace_started_)
        return;

    for (const auto& sv : enabled_categories)
        enabled_.emplace_back(sv);
    trace_started_ = true;
}

void SkEventTracerImpl::StopTracing()
{
    trace_started_ = false;
    enabled_.clear();
}

const uint8_t *SkEventTracerImpl::getCategoryGroupEnabled(const char *name)
{
    static uint8_t kYes = 1;
    static uint8_t kNo = 0;

    if (!trace_started_)
        return &kNo;

    for (const std::string_view& sv : utils::SplitString(name, ','))
    {
        HashString<std::string_view> hashed(sv);
        auto itr = std::find(enabled_.begin(), enabled_.end(), hashed);
        if (itr != enabled_.end())
            return &kYes;
    }

    return &kNo;
}

const char *SkEventTracerImpl::getCategoryGroupName(const uint8_t *categoryEnabledFlag)
{
    // TODO(sora): implement this
    return nullptr;
}

SkEventTracer::Handle SkEventTracerImpl::addTraceEvent(char phase,
                                                       const uint8_t *categoryEnabledFlag,
                                                       const char *name,
                                                       uint64_t id,
                                                       int32_t numArgs,
                                                       const char **argNames,
                                                       const uint8_t *argTypes,
                                                       const uint64_t *argValues,
                                                       uint8_t flags)
{
    // TODO(sora): Support other phase

    switch (phase)
    {
    case TRACE_EVENT_PHASE_COMPLETE:
    case TRACE_EVENT_PHASE_BEGIN:
        TRACE_EVENT_BEGIN("skia", perfetto::StaticString{name});
        break;
    case TRACE_EVENT_PHASE_END:
        TRACE_EVENT_END("skia");
        break;
    default:
        return 0;
    }

    return 1;
}

void SkEventTracerImpl::updateTraceEventDuration(const uint8_t *categoryEnabledFlag,
                                                 const char *name,
                                                 SkEventTracer::Handle handle)
{
    if (handle == 0)
        return;

    TRACE_EVENT_END("skia");
}

GLAMOR_NAMESPACE_END
