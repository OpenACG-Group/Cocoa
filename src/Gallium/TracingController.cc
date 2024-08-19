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

#include "src/tracing/trace-event.h"
// V8 defines a macro `CHECK` which conflicts with our.
#undef CHECK

#include "Core/TraceEvent.h"
#include "Core/Utils.h"
#include "Gallium/TracingController.h"
GALLIUM_NS_BEGIN

TracingController::TracingController()
    : trace_started_(false)
    , categories_{}
    , nb_categories_(0)
{
}

void TracingController::StartTracing()
{
    if (trace_started_)
        return;
    trace_started_ = true;
    // Reset the category cache
    nb_categories_ = 0;
}

void TracingController::StopTracing()
{
    trace_started_ = false;
}

const uint8_t *TracingController::GetCategoryGroupEnabled(const char *name)
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
    categories_[nb_categories_].enabled =
            CategoryGroupEnabledFlags::kEnabledForRecording_CategoryGroupEnabledFlags;
    categories_[nb_categories_].name = name;
    return reinterpret_cast<uint8_t*>(&categories_[nb_categories_++]);
}

const char *TracingController::GetCategoryName(const uint8_t *category_enabled_flag)
{
    if (category_enabled_flag)
        return reinterpret_cast<const CategoryState*>(category_enabled_flag)->name;
    return nullptr;
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
    perfetto::DynamicCategory category{ GetCategoryName(category_enabled_flag) };

    if (TRACE_EVENT_PHASE_COMPLETE == phase || TRACE_EVENT_PHASE_INSTANT == phase)
    {
        // TODO(sora): record the arguments and other attrs of tracing event
        TRACE_EVENT_BEGIN(category, nullptr, [&](perfetto::EventContext& ctx) {
            ctx.event()->set_name(name);
        });
    }
    else if (phase == TRACE_EVENT_PHASE_END)
        TRACE_EVENT_END(category);

    if (phase == TRACE_EVENT_PHASE_INSTANT)
        TRACE_EVENT_END(category);

    return 0;
}

void TracingController::UpdateTraceEventDuration(const uint8_t *category_enabled_flag,
                                                 const char *name, uint64_t handle)
{
    perfetto::DynamicCategory category{ GetCategoryName(category_enabled_flag) };
    TRACE_EVENT_END(category);
}

GALLIUM_NS_END
