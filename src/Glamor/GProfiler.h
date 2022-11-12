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

#ifndef COCOA_GLAMOR_GPROFILER_H
#define COCOA_GLAMOR_GPROFILER_H

#include <chrono>

#include "Glamor/Glamor.h"
GLAMOR_NAMESPACE_BEGIN

class GProfiler
{
public:
    constexpr static int32_t kRbMaxThreshold = 4096;
    constexpr static uint64_t kRbHeadFrame = 0;
    using Timepoint = std::chrono::steady_clock::time_point;

    enum FrameMilestone
    {
        kRequested_FrameMilestone = 0,
        kPresented_FrameMilestone,
        kPrerollBegin_FrameMilestone,
        kPrerollEnd_FrameMilestone,
        kPaintBegin_FrameMilestone,
        kPaintEnd_FrameMilestone,
        kBegin_FrameMilestone,
        kEnd_FrameMilestone,
        kLast_FrameMilestone
    };

    struct Report
    {
        using Ptr = std::unique_ptr<Report, std::function<void(Report*)>>;

        Timepoint timebase;
        size_t n_entries;
        struct Entry
        {
            uint64_t frame;
            Timepoint milestones[kLast_FrameMilestone];
        } entries[];
    };

    GProfiler();
    ~GProfiler();

    g_private_api void BeginFrame();
    g_private_api void EndFrame();

    g_private_api void MarkMilestoneInFrame(FrameMilestone milestone);

    g_locked_sync_api void PurgeRecentHistorySamples(bool free_memory);

    g_locked_sync_api Report::Ptr GenerateCurrentReport();

private:
    // NOLINTNEXTLINE
    struct Sample
    {
        bool alive;
        Timepoint timestamp[kLast_FrameMilestone + 1];
        uint64_t frame;
        bool pending;
        Sample *p_next;
        Sample *p_prev;
    };

    Sample *CreateNewSample();
    void PopFirstSample();

    std::mutex rb_lock_;
    Timepoint timebase_;
    size_t rb_samples_threshold_;
    std::vector<Sample*> sample_alloc_cache_;
    Sample *rb_head_;
    uint64_t frame_counter_;
    Sample *current_sample_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_GPROFILER_H
