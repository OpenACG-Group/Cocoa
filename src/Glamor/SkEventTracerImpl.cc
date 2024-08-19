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
#include "src/core/SkTraceEvent.h"

#include "Core/Utils.h"
#include "Core/TraceEvent.h"
#include "Glamor/SkEventTracerImpl.h"
GLAMOR_NAMESPACE_BEGIN

SkEventTracerImpl::SkEventTracerImpl()
    : trace_started_(false)
    , categories_{}
    , nb_categories_(0)
{
}

SkEventTracerImpl::~SkEventTracerImpl() = default;

void SkEventTracerImpl::StartTracing()
{
    if (trace_started_)
        return;
    trace_started_ = true;
    // Reset the category cache
    nb_categories_ = 0;
}

void SkEventTracerImpl::StopTracing()
{
    trace_started_ = false;
}

const uint8_t *SkEventTracerImpl::getCategoryGroupEnabled(const char *name)
{
    static uint8_t kNo = 0;
    if (!trace_started_)
        return &kNo;

    // We ignore the `disabled-by-default-` prefix
    if (utils::StrStartsWith(std::string_view(name),"disabled-by-default-"))
        name += std::char_traits<char>::length("disabled-by-default-");

    std::scoped_lock lock(lock_);
    for (int i = 0; i < nb_categories_; i++)
    {
        if (strcmp(name, categories_[i].name) == 0)
            return reinterpret_cast<uint8_t*>(&categories_[i]);
    }

    CHECK(nb_categories_ < kMaxCategories);

    // Append an entry
    categories_[nb_categories_].enabled = SkEventTracer::kEnabledForRecording_CategoryGroupEnabledFlags;
    categories_[nb_categories_].name = name;
    return reinterpret_cast<uint8_t*>(&categories_[nb_categories_++]);
}

const char *SkEventTracerImpl::getCategoryGroupName(const uint8_t *categoryEnabledFlag)
{
    if (categoryEnabledFlag)
        return reinterpret_cast<const CategoryState*>(categoryEnabledFlag)->name;
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
    perfetto::DynamicCategory category{ getCategoryGroupName(categoryEnabledFlag) };

    if (TRACE_EVENT_PHASE_COMPLETE == phase || TRACE_EVENT_PHASE_INSTANT == phase)
    {
        switch (numArgs)
        {
        case 0:
            TriggerTraceEvent(categoryEnabledFlag, name);
            break;
        case 1:
            TriggerTraceEvent(categoryEnabledFlag, name, argNames[0], argTypes[0], argValues[0]);
            break;
        case 2:
            TriggerTraceEvent(categoryEnabledFlag, name, argNames[0], argTypes[0], argValues[0],
                              argNames[1], argTypes[1], argValues[1]);
            break;
        default:
            break;
        }
    }
    else if (phase == TRACE_EVENT_PHASE_END)
    {
        TRACE_EVENT_END(category);
    }

    if (phase == TRACE_EVENT_PHASE_INSTANT)
    {
        TRACE_EVENT_END(category);
    }
    return 0;
}

void SkEventTracerImpl::updateTraceEventDuration(const uint8_t *categoryEnabledFlag,
                                                 const char *name,
                                                 SkEventTracer::Handle handle)
{
    // This is only ever called from a scoped trace event, so we will just end the event
    perfetto::DynamicCategory category{ getCategoryGroupName(categoryEnabledFlag) };
    TRACE_EVENT_END(category);
}

void SkEventTracerImpl::TriggerTraceEvent(const uint8_t *category_enabled_flag, const char *event_name)
{
    perfetto::DynamicCategory category{ getCategoryGroupName(category_enabled_flag) };
    TRACE_EVENT_BEGIN(category, nullptr, [&](perfetto::EventContext ctx) {
        ctx.event()->set_name(event_name);
    });
}

void SkEventTracerImpl::TriggerTraceEvent(const uint8_t *category_enabled_flag, const char *event_name,
                                          const char *arg1_name, const uint8_t& arg1_type, const uint64_t& arg1_val)
{
    perfetto::DynamicCategory category{ getCategoryGroupName(category_enabled_flag) };
    switch (arg1_type)
    {
    case TRACE_VALUE_TYPE_BOOL: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val,
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_UINT: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val,
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_INT: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, static_cast<int64_t>(arg1_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_DOUBLE: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, sk_bit_cast<double>(arg1_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_POINTER: {
        TRACE_EVENT_BEGIN(category, nullptr,
                          arg1_name, skia_private::TraceValueAsPointer(arg1_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_COPY_STRING: [[fallthrough]]; // Perfetto always copies string data
    case TRACE_VALUE_TYPE_STRING: {
        TRACE_EVENT_BEGIN(category, nullptr,
                          arg1_name, skia_private::TraceValueAsString(arg1_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    default: {
        MARK_UNREACHABLE();
    }
    }
}

namespace {

template<typename T>
void begin_event_with_second_arg(const char *category_name, const char *event_name,
                                 const char *arg1_name, T arg1_val, const char *arg2_name,
                                 const uint8_t& arg2_type, const uint64_t& arg2_val)
{
    perfetto::DynamicCategory category{category_name};

    switch (arg2_type)
    {
    case TRACE_VALUE_TYPE_BOOL: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val, arg2_name, arg2_val,
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_UINT: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val, arg2_name, arg2_val,
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_INT: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val,
                          arg2_name, static_cast<int64_t>(arg2_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_DOUBLE: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val,
                          arg2_name, sk_bit_cast<double>(arg2_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_POINTER: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val,
                          arg2_name, skia_private::TraceValueAsPointer(arg2_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    case TRACE_VALUE_TYPE_COPY_STRING: [[fallthrough]];
    case TRACE_VALUE_TYPE_STRING: {
        TRACE_EVENT_BEGIN(category, nullptr, arg1_name, arg1_val,
                          arg2_name, skia_private::TraceValueAsString(arg2_val),
                          [&](perfetto::EventContext ctx) {
                          ctx.event()->set_name(event_name); });
        break;
    }
    default: {
        MARK_UNREACHABLE();
        break;
    }
    }
}

} // namespace anonymous

void SkEventTracerImpl::TriggerTraceEvent(const uint8_t *category_enabled_flag, const char *event_name,
                                          const char *arg1_name, const uint8_t& arg1_type, const uint64_t& arg1_val,
                                          const char *arg2_name, const uint8_t& arg2_type, const uint64_t& arg2_val)
{
    const char * category{ this->getCategoryGroupName(category_enabled_flag) };

    switch (arg1_type)
    {
    case TRACE_VALUE_TYPE_BOOL: {
        begin_event_with_second_arg(category, event_name, arg1_name, static_cast<bool>(arg1_val),
                                    arg2_name, arg2_type, arg2_val);
        break;
    }
    case TRACE_VALUE_TYPE_UINT: {
        begin_event_with_second_arg(category, event_name, arg1_name, arg1_val,
                                    arg2_name, arg2_type, arg2_val);
        break;
    }
    case TRACE_VALUE_TYPE_INT: {
        begin_event_with_second_arg(category, event_name,
                                    arg1_name, static_cast<int64_t>(arg1_val),
                                    arg2_name, arg2_type, arg2_val);
        break;
    }
    case TRACE_VALUE_TYPE_DOUBLE: {
        begin_event_with_second_arg(category, event_name, arg1_name, sk_bit_cast<double>(arg1_val),
                                    arg2_name, arg2_type, arg2_val);
        break;
    }
    case TRACE_VALUE_TYPE_POINTER: {
        begin_event_with_second_arg(category, event_name,
                                    arg1_name, skia_private::TraceValueAsPointer(arg1_val),
                                    arg2_name, arg2_type, arg2_val);
        break;
    }
    case TRACE_VALUE_TYPE_COPY_STRING: [[fallthrough]];
    case TRACE_VALUE_TYPE_STRING: {
        begin_event_with_second_arg(category, event_name,
                                    arg1_name, skia_private::TraceValueAsString(arg1_val),
                                    arg2_name, arg2_type, arg2_val);
        break;
    }
    default: {
        MARK_UNREACHABLE();
    }
    }
}

GLAMOR_NAMESPACE_END
