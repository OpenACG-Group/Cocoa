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

#ifndef COCOA_GLAMOR_SKEVENTTRACERIMPL_H
#define COCOA_GLAMOR_SKEVENTTRACERIMPL_H

#include <string>
#include <string_view>

#include "include/utils/SkEventTracer.h"

#include "Core/HashString.h"
#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class SkEventTracerImpl : public SkEventTracer
{
public:
    SkEventTracerImpl();
    ~SkEventTracerImpl() override;

    const char *getCategoryGroupName(const uint8_t *categoryEnabledFlag) override;
    const uint8_t *getCategoryGroupEnabled(const char *name) override;

    SkEventTracer::Handle addTraceEvent(char phase,
                                        const uint8_t *categoryEnabledFlag,
                                        const char *name,
                                        uint64_t id,
                                        int32_t numArgs,
                                        const char **argNames,
                                        const uint8_t *argTypes,
                                        const uint64_t *argValues,
                                        uint8_t flags) override;

    void updateTraceEventDuration(const uint8_t *categoryEnabledFlag,
                                  const char *name,
                                  SkEventTracer::Handle handle) override;

    void StartTracing(const std::vector<std::string>& enabled_categories);
    void StopTracing();

private:
    bool                                     trace_started_;
    std::vector<HashString<std::string>>     enabled_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_SKEVENTTRACERIMPL_H
