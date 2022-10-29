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

#ifndef COCOA_GLAMOR_MAYBEGPUOBJECT_H
#define COCOA_GLAMOR_MAYBEGPUOBJECT_H

#include <type_traits>
#include <list>

#include "include/core/SkRefCnt.h"

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/RenderHostTaskRunner.h"
#include "Glamor/RenderHost.h"
#include "Glamor/GraphicsResourcesTrackable.h"
GLAMOR_NAMESPACE_BEGIN

class MaybeGpuObjectBase;

/**
 * All the methods are thread-safe.
 */
class GpuThreadSharedObjectsCollector : public GraphicsResourcesTrackable
{
public:
    GpuThreadSharedObjectsCollector() = default;
    ~GpuThreadSharedObjectsCollector() override;

    void AddAliveObject(MaybeGpuObjectBase *ptr);
    void DeleteDeadObject(MaybeGpuObjectBase *ptr);

    void Collect();
    void Trace(Tracer *tracer) noexcept override;

private:
    std::mutex                     list_lock_;
    std::list<MaybeGpuObjectBase*> alive_objects_;
};

class MaybeGpuObjectBase
{
    // Allow calling `ForceCollect`
    friend class GpuThreadSharedObjectsCollector;

public:
    using CollectedCallback = std::function<void()>;

    MaybeGpuObjectBase(bool isRetained, SkRefCnt *ptr, RenderHost *renderHost);
    MaybeGpuObjectBase(const MaybeGpuObjectBase& other);
    MaybeGpuObjectBase(MaybeGpuObjectBase&& rhs) noexcept;
    ~MaybeGpuObjectBase();

    void SetObjectCollectedCallback(const CollectedCallback& cb) {
        collected_callback_ = cb;
    }

    void ResetObjectCollectedCallback() {
        collected_callback_ = {};
    }

protected:
    void InternalReset(bool isRetained, SkRefCnt *ptr, RenderHost *renderHost,
                       bool should_remove_entry = true);
    void ForceCollect();

private:
    bool                 is_retained_;
    SkRefCnt            *object_;
    RenderHost          *render_host_;
    CollectedCallback    collected_callback_;
};

/**
 * Some Skia objects keep references to GPU resources directly (like SkImage)
 * or indirectly (like SkPicture). For those objects, they must be destructed
 * on GPU thread to avoid unpredictable synchronization error.
 * Objects which hold GPU resources are always reference-counted (derived from SkRefCnt),
 * and there are possibilities that they may be destructed by `unref()` method.
 * Wrapping those objects into a `MaybeGpuObject<T>` is an efficient way
 * to solve that problem. Wrapped objects will be destructed by `RenderHostTaskRunner`
 * which runs in GPU thread if necessary.
 * Note that even though a `MaybeGpuObject<T>` object, it still may be destructed
 * on current thread locally if it was constructed with `isRetained` being false.
 */
template<typename T>
class MaybeGpuObject : public MaybeGpuObjectBase
{
public:
    CO_NONASSIGNABLE(MaybeGpuObject<T>)

    using element_type = T;
    static_assert(std::is_base_of<SkRefCnt, element_type>::value, "typecheck: Object is not inherited from SkRefCnt");

    MaybeGpuObject()
        : MaybeGpuObjectBase(false, nullptr, nullptr), ptr_(nullptr) {}

    // NOLINTNEXTLINE
    MaybeGpuObject(std::nullptr_t)
        : MaybeGpuObjectBase(false, nullptr, nullptr), ptr_(nullptr) {}

    MaybeGpuObject(bool isRetained, const sk_sp<element_type>& sp, RenderHost *host = nullptr)
        : MaybeGpuObjectBase(isRetained, sp.get(), host), ptr_(sp.get()) {}

    MaybeGpuObject(bool isRetained, element_type *barePtr, RenderHost *host = nullptr)
        : MaybeGpuObjectBase(isRetained, barePtr, host), ptr_(barePtr) {}

    template<typename R, typename std::enable_if_t<std::is_convertible_v<T*, R*>>>
    explicit MaybeGpuObject(const MaybeGpuObject<R>& other)
        : MaybeGpuObjectBase(other), ptr_(static_cast<element_type>(other.ptr_)) {}

    ~MaybeGpuObject() = default;

    void reset(bool isRetained = false, element_type *ptr = nullptr, RenderHost *host = nullptr) {
        InternalReset(isRetained, ptr, host);
        ptr_ = ptr;
    }

    g_nodiscard element_type *Get() const {
        return ptr_;
    }

    g_nodiscard element_type& operator*() const {
        CHECK(ptr_ && "dereference a null pointer is illegal");
        return *ptr_;
    }

    g_nodiscard element_type* operator->() const {
        CHECK(ptr_ && "dereference a null pointer is illegal");
        return ptr_;
    }

    g_nodiscard bool operator==(std::nullptr_t) const {
        return (ptr_ == nullptr);
    }

    // NOLINTNEXTLINE
    g_nodiscard operator bool() const {
        return !(ptr_ == nullptr);
    }

private:
    element_type    *ptr_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MAYBEGPUOBJECT_H
