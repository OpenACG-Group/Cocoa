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

#ifndef COCOA_GALLIUM_TRACINGCONTROLLER_H
#define COCOA_GALLIUM_TRACINGCONTROLLER_H

#include "include/v8-platform.h"

#include "Core/HashString.h"
#include "Gallium/Gallium.h"
GALLIUM_NS_BEGIN

class TracingController : public v8::TracingController
{
public:
    TracingController();
    ~TracingController() override = default;

    void StartTracing();
    void StopTracing();

    const uint8_t *GetCategoryGroupEnabled(const char *name) override;

    uint64_t AddTraceEvent(char phase,
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
                           unsigned int flags) override;

    /*
    uint64_t AddTraceEventWithTimestamp(char phase,
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
                                        int64_t timestamp) override;
    */

    void UpdateTraceEventDuration(const uint8_t *category_enabled_flag,
                                  const char *name, uint64_t handle) override;

private:
    const char *GetCategoryName(const uint8_t *category_enabled_flag);

    constexpr static int kMaxCategories = 256;

    struct CategoryState
    {
        uint8_t enabled;
        const char *name;
    };

    std::mutex        lock_;
    bool              trace_started_;
    CategoryState     categories_[kMaxCategories];
    int               nb_categories_;
};

GALLIUM_NS_END
#endif //COCOA_GALLIUM_TRACINGCONTROLLER_H
