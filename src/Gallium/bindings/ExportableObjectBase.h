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

#ifndef COCOA_GALLIUM_BINDINGS_EXPORTABLEOBJECTBASE_H
#define COCOA_GALLIUM_BINDINGS_EXPORTABLEOBJECTBASE_H

#include "include/v8.h"

#include "Core/Errors.h"
#include "Gallium/Gallium.h"
GALLIUM_BINDINGS_NS_BEGIN

/**
 * Base class of exportable C++ classes.
 * Every class that will be exported to JavaScript via `binder::Class`
 * must inherit this class. This can be treated as a simplified
 * reflection mechanism, allowing some bindings to access, transfer
 * and clone JavaScript objects exported from C++ without knowing
 * their certain types.
 */
class ExportableObjectBase
{
public:
    /**
     * A set of object attributes describes the object itself.
     */
    enum ObjectAttributes
    {
        /**
         * Object is transferable.
         * Similar to the move semantic in C++, datas can be "stole out"
         * from a transferable object instance, and then datas are "filled"
         * into a new object instance, making the original object become
         * invalid.
         * All the members in a transferable object must be transferable.
         */
        kTransferable_Attr = 0x01,

        /**
         * Object is cloneable.
         * Similar to the copy semantic in C++, datas of a cloneable object
         * can be copied to create a new object instance.
         * ALl the members in a cloneable object must be cloneable.
         */
        kCloneable_Attr = 0x02,

        // For `MessagePortWrap` of `workers` binding only
        kMessagePort_Attr = 0x04
    };

    class FlattenedData
    {
    public:
        virtual ~FlattenedData() = default;
        virtual v8::MaybeLocal<v8::Object> Deserialize(
                v8::Isolate *isolate, v8::Local<v8::Context> context) = 0;
    };

    using MaybeFlattened = v8::Maybe<std::shared_ptr<FlattenedData>>;

    /**
     * A function that serializes the given object `base`, and returns a
     * `v8::Just<std::shared_ptr<FlattenedData>>(...)` if succeeds.
     * If `pretest` is `true`, the function should not serialize any object,
     * just return `FlattenPretestResult(true)` to indicate that the given
     * object can be transferred or cloned (it has not been transferred to
     * other Isolates), otherwise, return `FlattenPretestResult(false)`.
     */
    using SerializerFunc = MaybeFlattened(*)(
            v8::Isolate *isolate, ExportableObjectBase *base, bool pretest);

    static MaybeFlattened FlattenPretestResult(bool ok) {
        if (ok)
            return v8::Just<std::shared_ptr<FlattenedData>>(nullptr);
        else
            return v8::Nothing<std::shared_ptr<FlattenedData>>();
    }

    static MaybeFlattened JustFlattened(const std::shared_ptr<FlattenedData>& data) {
        return v8::Just<std::shared_ptr<FlattenedData>>(data);
    }

    class Descriptor
    {
    public:
        Descriptor(ExportableObjectBase *base, uint32_t attrs)
                : base_(base), attributes_(attrs) {}

        g_nodiscard g_inline bool IsTransferable() const {
            return (attributes_ & kTransferable_Attr);
        }

        g_nodiscard g_inline bool IsCloneable() const {
            return (attributes_ & kCloneable_Attr);
        }

        g_nodiscard g_inline bool IsMessagePort() const {
            return (attributes_ & kMessagePort_Attr);
        }

        g_nodiscard g_inline ExportableObjectBase *GetBase() const {
            return base_;
        }

        // Set by `binder::ObjectRegistry::wrap_object()`
        g_private_api void SetObjectWeakReference(v8::Isolate *isolate, v8::Local<v8::Object> self) {
            base_->self_weak_.Reset(isolate, self);
            base_->self_weak_.SetWeak();
            if (base_->on_object_weak_ref_valid_)
                base_->on_object_weak_ref_valid_();
        }

        g_nodiscard g_inline SerializerFunc GetTransferSerializer() const {
            return base_->pfn_transfer_serializer_;
        }

        g_nodiscard g_inline SerializerFunc GetCloneSerializer() const {
            return base_->pfn_clone_serializer_;
        }

    private:
        ExportableObjectBase   *base_;
        uint32_t                attributes_;
    };

    explicit ExportableObjectBase(
        uint32_t attrs = 0,
        std::function<void(void)> on_object_weak_ref_valid = {},
        SerializerFunc pfn_transfer_serializer = nullptr,
        SerializerFunc pfn_clone_serializer = nullptr
    )   : descriptor_(this, attrs)
        , on_object_weak_ref_valid_(std::move(on_object_weak_ref_valid))
        , pfn_transfer_serializer_(pfn_transfer_serializer)
        , pfn_clone_serializer_(pfn_clone_serializer)
    {
        if (attrs & kTransferable_Attr)
            CHECK(pfn_transfer_serializer);
        if (attrs & kCloneable_Attr)
            CHECK(pfn_clone_serializer);
    }

    template<typename T>
    T *Cast() {
        static_assert(std::is_base_of<ExportableObjectBase, T>::value);
        // NOLINTNEXTLINE
        return static_cast<T*>(this);
    }

    /**
     * Get the unique descriptor of the object.
     * Attributes accessing, object cloning/transferring are implemented
     * with the help of descriptor. Returned pointer is valid during the
     * whole lifetime of the object.
     * Descriptor also could be got from the JavaScript handle of the object
     * by using `binder::UnwrapObjectDescriptor()`.
     */
    g_nodiscard g_inline Descriptor *GetObjectDescriptor() {
        return &descriptor_;
    }

    /**
     * Get the weak-referenced handle of the object itself.
     * Our memory-model keeps this weak reference valid during the whole lifetime
     * of this object. As long as the object has not been destructed, the weak
     * handle is valid.
     *
     * As the returned handle refers to the object itself, if you convert it to
     * a strong-referenced `v8::Global` or `v8::Persistent` handle, and store it
     * directly or indirectly as a member of the object, the object will never be
     * collected by GC (because it holds a reference to itself). It will be freed
     * when `binder::Cleanup()` is called (when `RuntimeBase::Dispose` is called).
     *
     * Only after `on_object_weak_ref_valid` callback given from constructor is
     * called, could you call this method. Otherwise, it crashes the program.
     */
    g_nodiscard g_inline const v8::Global<v8::Object>& GetObjectWeakReference() const {
        CHECK(!self_weak_.IsEmpty());
        return self_weak_;
    }

private:
    Descriptor                  descriptor_;
    v8::Global<v8::Object>      self_weak_;
    std::function<void(void)>   on_object_weak_ref_valid_;
    SerializerFunc              pfn_transfer_serializer_;
    SerializerFunc              pfn_clone_serializer_;
};

GALLIUM_BINDINGS_NS_END
#endif //COCOA_GALLIUM_BINDINGS_EXPORTABLEOBJECTBASE_H
