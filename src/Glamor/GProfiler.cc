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

#include "fmt/format.h"

#include "Core/Journal.h"
#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/GProfiler.h"
GLAMOR_NAMESPACE_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Glamor.GProfiler)

GProfiler::GProfiler()
    : timebase_(std::chrono::steady_clock::now())
    , rb_samples_threshold_(0)
    , rb_head_(nullptr)
    , frame_counter_(1)
    , current_sample_(nullptr)
{
    CHECK(GlobalScope::Instance());
    rb_samples_threshold_ = GlobalScope::Ref()
                            .GetOptions()
                            .GetProfilerRingBufferThreshold();
    if (rb_samples_threshold_ > kRbMaxThreshold)
    {
        QLOG(LOG_WARNING, "The ring buffer threshold set by user ({}) is out of range, "
                          "resetting to default value {}",
                          rb_samples_threshold_,
                          GLAMOR_PROFILER_RINGBUFFER_THRESHOLD_DEFAULT);

        rb_samples_threshold_ = GLAMOR_PROFILER_RINGBUFFER_THRESHOLD_DEFAULT;
        GlobalScope::Ref().GetOptions()
            .SetProfilerRingBufferThreshold(rb_samples_threshold_);
    }

    rb_head_ = new Sample();
    CHECK(rb_head_);

    rb_head_->alive = true;
    rb_head_->pending = false;
    rb_head_->frame = kRbHeadFrame;
    rb_head_->p_next = rb_head_;
    rb_head_->p_prev = rb_head_;

    sample_alloc_cache_.push_back(rb_head_);
}

GProfiler::~GProfiler()
{
    for (Sample *ptr : sample_alloc_cache_)
        delete ptr;
    sample_alloc_cache_.clear();
}

GProfiler::Sample *GProfiler::CreateNewSample()
{
    if (sample_alloc_cache_.size() > rb_samples_threshold_)
        PopFirstSample();

    Sample *current = nullptr;
    for (Sample *ptr : sample_alloc_cache_)
    {
        if (!ptr->alive)
        {
            current = ptr;
            break;
        }
    }
    if (!current)
    {
        current = new Sample;
        sample_alloc_cache_.push_back(current);
    }
    CHECK(current);

    current->p_next = rb_head_;
    current->p_prev = rb_head_->p_prev;
    rb_head_->p_prev->p_next = current;
    rb_head_->p_prev = current;
    rb_head_->pending = true;

    current->alive = true;
    current->frame = frame_counter_++;
    return current;
}

void GProfiler::PopFirstSample()
{
    if (rb_head_->p_next == rb_head_)
        return;

    rb_head_->p_next->alive = false;
    rb_head_->p_next->p_next->p_prev = rb_head_;
    rb_head_->p_next = rb_head_->p_next->p_next;
}

void GProfiler::BeginFrame()
{
    CHECK(!current_sample_);
    std::scoped_lock<std::mutex> lock(rb_lock_);
    current_sample_ = CreateNewSample();
    MarkMilestoneInFrame(kBegin_FrameMilestone);
}

void GProfiler::EndFrame()
{
    CHECK(current_sample_);
    MarkMilestoneInFrame(kEnd_FrameMilestone);

    std::scoped_lock<std::mutex> lock(rb_lock_);
    current_sample_->pending = false;
    current_sample_ = nullptr;
}

void GProfiler::PurgeRecentHistorySamples(bool free_memory)
{
    std::scoped_lock<std::mutex> lock(rb_lock_);

    if (rb_head_->p_next == rb_head_)
        return;

    rb_head_->p_next = rb_head_;
    rb_head_->p_prev = rb_head_;
    for (Sample *ptr : sample_alloc_cache_)
    {
        if (ptr != rb_head_)
        {
            if (free_memory)
                delete ptr;
            else
                ptr->alive = false;
        }
    }

    if (free_memory)
    {
        sample_alloc_cache_.clear();
        sample_alloc_cache_.push_back(rb_head_);
    }
}

void GProfiler::MarkMilestoneInFrame(FrameMilestone milestone)
{
    CHECK(milestone < kLast_FrameMilestone);
    CHECK(current_sample_);
    current_sample_->timestamp[milestone] = std::chrono::steady_clock::now();
}

GProfiler::Report::Ptr GProfiler::GenerateCurrentReport()
{
    std::scoped_lock<std::mutex> lock(rb_lock_);
    if (rb_head_->p_next == rb_head_)
    {
        // No samples
        return nullptr;
    }

    // Last valid sample
    Sample *lvs = rb_head_->p_prev;
    while (lvs->pending)
    {
        lvs = lvs->p_prev;
    }

    if (lvs == rb_head_)
    {
        // No valid samples
        return nullptr;
    }

    size_t n_entries = lvs->frame - rb_head_->p_next->frame + 1;
    CHECK(n_entries >= 1);

    size_t pod_size = sizeof(Report) + n_entries * sizeof(Report::Entry);
    auto *report = reinterpret_cast<Report*>(std::malloc(pod_size));
    CHECK(report);

    report->timebase = timebase_;
    report->n_entries = n_entries;

    Sample *cur = rb_head_->p_next;
    Report::Entry *p_entry = report->entries;
    while(true)
    {
        p_entry->frame = cur->frame;
        std::copy(&cur->timestamp[0],
                  &cur->timestamp[kLast_FrameMilestone],
                  &p_entry->milestones[0]);

        if (cur == lvs)
            break;

        cur = cur->p_next;
        p_entry++;
    }

    return {report, [](Report *ptr) {
        CHECK(ptr);
        std::free(ptr);
    }};
}

GLAMOR_NAMESPACE_END
