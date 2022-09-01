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

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/MaybeGpuObject.h"
GLAMOR_NAMESPACE_BEGIN

MaybeGpuObjectBase::MaybeGpuObjectBase(bool isRetained, SkRefCnt *ptr, RenderHost *renderHost)
    : is_retained_(isRetained)
    , object_(ptr)
    , render_host_(renderHost)
{
    if (ptr == nullptr)
    {
        // nullptr is not retained by anyone
        is_retained_  = false;
        render_host_ = nullptr;
        return;
    }
    else
    {
        ptr->ref();
    }

    // If the object is not retained by GPU thread, there is no need to
    // get the `RenderHost` object.
    if (render_host_ == nullptr && is_retained_)
    {
        render_host_ = GlobalScope::Ref().GetRenderHost();
        CHECK(render_host_ && "invalid RenderHost");
    }
}

MaybeGpuObjectBase::MaybeGpuObjectBase(const MaybeGpuObjectBase& other)
    : is_retained_(other.is_retained_)
    , object_(other.object_)
    , render_host_(other.render_host_)
{
    if (object_ != nullptr)
    {
        object_->ref();
    }
}

MaybeGpuObjectBase::MaybeGpuObjectBase(MaybeGpuObjectBase&& rhs) noexcept
    : is_retained_(rhs.is_retained_)
    , object_(rhs.object_)
    , render_host_(rhs.render_host_)
{
    if (object_ != nullptr)
    {
        object_->ref();
        rhs.InternalReset(false, nullptr, nullptr);
    }
}

MaybeGpuObjectBase::~MaybeGpuObjectBase()
{
    InternalReset(false, nullptr, nullptr);
}

namespace {

void taskrunner_release_callback(RenderHostCallbackInfo& info)
{
    // Do nothing.
}

} // namespace anonymous

void MaybeGpuObjectBase::InternalReset(bool isRetained, SkRefCnt *ptr, RenderHost *renderHost)
{
    if (is_retained_)
    {
        // `object_` is retained by GPU thread then it must be released in GPU thread.
        // Task runner of GPU thread can help us to do that.
        CHECK(render_host_ && "invalid RenderHost");
        const auto& taskRunner = render_host_->GetRenderHostTaskRunner();
        RenderClientCallInfo callInfo(GLOP_TASKRUNNER_RUN);
        callInfo.EmplaceBack<RenderHostTaskRunner::Task>([inner_ptr = object_]() {
            inner_ptr->unref();
        });
        taskRunner->Invoke(std::move(callInfo), taskrunner_release_callback);
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
        render_host_ = nullptr;
        return;
    }

    is_retained_ = isRetained;
    render_host_ = renderHost;
    if (render_host_ == nullptr && is_retained_)
    {
        render_host_ = GlobalScope::Ref().GetRenderHost();
        CHECK(render_host_ && "invalid RenderHost");
    }
}

GLAMOR_NAMESPACE_END
