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

#include <unordered_map>

#include "Gallium/bindings/glamor/Exports.h"
#include "Glamor/GProfiler.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

void GProfilerWrap::purgeRecentHistorySamples(bool free_memory)
{
    profiler_->PurgeRecentHistorySamples(free_memory);
}

namespace {

std::unordered_map<gl::GProfiler::FrameMilestone, std::string_view> g_frame_milestones_tags = {
    { gl::GProfiler::kRequested_FrameMilestone,     "requested"     },
    { gl::GProfiler::kPresented_FrameMilestone,     "presented"     },
    { gl::GProfiler::kPrerollBegin_FrameMilestone,  "prerollBegin"  },
    { gl::GProfiler::kPrerollEnd_FrameMilestone,    "prerollEnd"    },
    { gl::GProfiler::kPaintBegin_FrameMilestone,    "paintBegin"    },
    { gl::GProfiler::kPaintEnd_FrameMilestone,      "paintEnd"      },
    { gl::GProfiler::kBegin_FrameMilestone,         "begin"         },
    { gl::GProfiler::kEnd_FrameMilestone,           "end"           }
};

} // namespace anonymous

v8::Local<v8::Value> GProfilerWrap::generateCurrentReport()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    gl::GProfiler::Report::Ptr report = profiler_->GenerateCurrentReport();
    if (!report)
        g_throw(Error, "No samples can be reported");

    namespace TM = std::chrono;

    TM::steady_clock::time_point tbase = report->timebase;

    std::vector<v8::Local<v8::Value>> entries(report->n_entries);
    for (int i = 0; i < report->n_entries; i++)
    {
        std::map<std::string_view, v8::Local<v8::Value>> entry;
        entry["frame"] = binder::to_v8(isolate, report->entries[i].frame);

        std::map<std::string_view, uint64_t> milestones;
        for (int tag = 0; tag < gl::GProfiler::kLast_FrameMilestone; tag++)
        {
            const std::string_view& tag_name = g_frame_milestones_tags[
                    static_cast<gl::GProfiler::FrameMilestone>(tag)];
            CHECK(!tag_name.empty());

            milestones[tag_name] = TM::duration_cast<TM::microseconds>(
                    report->entries[i].milestones[tag] - tbase).count();
        }

        entry["milestones"] = binder::to_v8(isolate, milestones);
        entries[i] = binder::to_v8(isolate, entry);
    }

    std::map<std::string_view, v8::Local<v8::Value>> result;
    result["timebaseUsSinceEpoch"] =
            binder::to_v8(isolate, TM::duration_cast<TM::microseconds>(
                    tbase.time_since_epoch()).count());
    result["entries"] = binder::to_v8(isolate, entries);

    return binder::to_v8(isolate, result);
}

GALLIUM_BINDINGS_GLAMOR_NS_END
