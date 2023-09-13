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

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/MaybeGpuObject.h"
#include "Glamor/PresentThread.h"
GLAMOR_NAMESPACE_BEGIN

void RemoteDestroyablesCollector::AddAliveObject(MaybeGpuObjectBase *ptr)
{
    std::scoped_lock<std::mutex> lock(list_lock_);
    auto itr = std::find(alive_objects_.begin(), alive_objects_.end(), ptr);
    if (itr != alive_objects_.end())
        return;
    alive_objects_.push_back(ptr);
}

void RemoteDestroyablesCollector::DeleteDeadObject(MaybeGpuObjectBase *ptr)
{
    std::scoped_lock<std::mutex> lock(list_lock_);
    auto itr = std::find(alive_objects_.begin(), alive_objects_.end(), ptr);
    if (itr == alive_objects_.end())
        return;
    alive_objects_.erase(itr);
}

void RemoteDestroyablesCollector::Collect()
{
    std::scoped_lock<std::mutex> lock(list_lock_);
    if (alive_objects_.empty())
        return;

    for (const auto& object : alive_objects_)
    {
        // Force the object to be collected (invalidated).
        // The owner will be informed that the object has been invalidated
        // if it registered a callback function.
        object->ForceCollect();
    }

    alive_objects_.clear();
}

RemoteDestroyablesCollector::~RemoteDestroyablesCollector()
{
    // `Collect` must be invoked explicitly before destructing.
    // This is guaranteed by `GlobalContext::Dispose`.
    CHECK(alive_objects_.empty());
}

void RemoteDestroyablesCollector::Trace(Tracer *tracer) noexcept
{
    std::scoped_lock<std::mutex> lock(list_lock_);

    int idx = 0;
    for (const auto& obj : alive_objects_)
    {
        tracer->TraceResource(fmt::format("ThreadSharedObject#{}", idx++),
                              TRACKABLE_TYPE_CLASS_OBJECT,
                              TRACKABLE_DEVICE_GPU,
                              TRACKABLE_OWNERSHIP_WEAK,
                              TraceIdFromPointer(obj->object_));
    }
}

MaybeGpuObjectBase::MaybeGpuObjectBase(bool isRetained, SkRefCnt *ptr, PresentThread *thread)
    : is_retained_(isRetained)
    , object_(ptr)
    , present_thread_(thread)
{
    if (ptr == nullptr)
    {
        // nullptr is not retained by anyone
        is_retained_  = false;
        present_thread_ = nullptr;
        return;
    }
    else
    {
        ptr->ref();
    }

    if (!present_thread_ && is_retained_)
    {
        present_thread_ = GlobalScope::Ref().GetPresentThread();
        CHECK(present_thread_ && "invalid PresentThread");
    }

    if (is_retained_)
    {
        // Construction: to create a new alive object
        auto *collector = GlobalScope::Ref()
                .GetPresentThread()->GetRemoteDestroyablesCollector();
        collector->AddAliveObject(this);
    }
}

MaybeGpuObjectBase::MaybeGpuObjectBase(const MaybeGpuObjectBase& other)
    : is_retained_(other.is_retained_)
    , object_(other.object_)
    , present_thread_(other.present_thread_)
{
    if (object_ != nullptr)
    {
        object_->ref();
    }

    if (is_retained_)
    {
        // Copy-semantic: to leave source object untouched,
        //                and create a new alive object
        auto *collector = GlobalScope::Ref()
                .GetPresentThread()->GetRemoteDestroyablesCollector();
        CHECK(collector);
        collector->AddAliveObject(this);
    }
}

MaybeGpuObjectBase::MaybeGpuObjectBase(MaybeGpuObjectBase&& rhs) noexcept
    : is_retained_(rhs.is_retained_)
    , object_(rhs.object_)
    , present_thread_(rhs.present_thread_)
{

    // Move-semantic: to delete the previous object and create a
    //                new alive object who takes over the ownership

    auto *collector = GlobalScope::Ref()
            .GetPresentThread()->GetRemoteDestroyablesCollector();
    CHECK(collector);

    if (object_ != nullptr)
    {
        object_->ref();
        rhs.InternalReset(false, nullptr, nullptr);
    }

    if (is_retained_)
        collector->AddAliveObject(this);
}

MaybeGpuObjectBase::~MaybeGpuObjectBase()
{
    InternalReset(false, nullptr, nullptr);
}

void MaybeGpuObjectBase::InternalReset(bool isRetained, SkRefCnt *ptr,
                                       PresentThread *thread,
                                       bool should_remove_entry)
{
    auto *collector = GlobalScope::Ref()
            .GetPresentThread()->GetRemoteDestroyablesCollector();

    if (is_retained_)
    {
        CHECK(collector);

        if (should_remove_entry)
            collector->DeleteDeadObject(this);

        // `object_` is retained by GPU thread then it must be released in GPU thread.
        // Task runner of GPU thread can help us to do that.
        CHECK(thread && "Invalid PresentThread");
        thread->SubmitTaskNoRet([inner_ptr = object_] { inner_ptr->unref(); }, {}, {});
    }
    else if (object_)
    {
        // `object_` is not retained by GPU thread, so we can release it in current thread.
        object_->unref();
    }

    object_ = ptr;
    if (object_ == nullptr)
    {
        // nullptr is not retained by anyone
        is_retained_ = false;
        present_thread_ = nullptr;
        return;
    }

    is_retained_ = isRetained;
    present_thread_ = thread;
    if (!present_thread_ && is_retained_)
    {
        present_thread_ = GlobalScope::Ref().GetPresentThread();
        CHECK(present_thread_ && "invalid PresentThread");
    }

    if (is_retained_)
    {
        CHECK(collector);
        collector->AddAliveObject(this);
    }
}

void MaybeGpuObjectBase::ForceCollect()
{
    if (collected_callback_)
        collected_callback_();

    InternalReset(false, nullptr, nullptr, false);
}

GLAMOR_NAMESPACE_END
