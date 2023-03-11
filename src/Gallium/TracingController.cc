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

#include "Core/TraceEvent.h"
#include "Core/Utils.h"
#include "Gallium/TracingController.h"
GALLIUM_NS_BEGIN

TracingController::TracingController()
    : tracing_started_(false)
{
}

void TracingController::StartTracing(const std::vector<std::string>& enabled)
{
    if (tracing_started_)
        return;

    for (const auto& sv : enabled)
        enabled_.emplace_back(sv);

    tracing_started_ = true;
}

void TracingController::StopTracing()
{
    tracing_started_ = false;
    enabled_.clear();
}

const uint8_t *TracingController::GetCategoryGroupEnabled(const char *name)
{
    static uint8_t kYes = 1;
    static uint8_t kNo = 0;

    if (!tracing_started_)
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

uint64_t TracingController::AddTraceEvent(char phase,
                                          const uint8_t *category_enabled_flag,
                                          const char *name,
                                          const char *scope,
                                          uint64_t id,
                                          uint64_t bind_id,
                                          int32_t num_args,
                                          const char **arg_names,
                                          const uint8_t *arg_types,
                                          const uint64_t *arg_values,
                                          std::unique_ptr<v8::ConvertableToTraceFormat> *arg_convertables,
                                          unsigned int flags)
{
    // TODO(sora): Support other phase

    switch (phase)
    {
    case 'X':   // COMPLETE
    case 'B':   // BEGIN
        TRACE_EVENT_BEGIN("v8", perfetto::StaticString{name});
        break;
    case 'E':   // END
        TRACE_EVENT_END("v8");
        break;
    default:
        return 0;
    }

    return 1;
}

uint64_t TracingController::AddTraceEventWithTimestamp(char phase,
                                                       const uint8_t *category_enabled_flag,
                                                       const char *name,
                                                       const char *scope,
                                                       uint64_t id,
                                                       uint64_t bind_id,
                                                       int32_t num_args,
                                                       const char **arg_names,
                                                       const uint8_t *arg_types,
                                                       const uint64_t *arg_values,
                                                       std::unique_ptr<v8::ConvertableToTraceFormat> *arg_convertables,
                                                       unsigned int flags,
                                                       int64_t timestamp)
{
    // FIXME(sora): It is necessary to implement this?
    return 0;
}

void TracingController::UpdateTraceEventDuration(const uint8_t *category_enabled_flag,
                                                 const char *name,
                                                 uint64_t handle)
{
    if (handle == 0)
        return;

    TRACE_EVENT_END("v8");
}

GALLIUM_NS_END
