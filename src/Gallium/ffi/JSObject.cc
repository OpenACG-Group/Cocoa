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

#include "Gallium/ffi/ClassRegistry.h"
#include "Gallium/ffi/JSObject.h"
GALLIUM_FFI_NS_BEGIN

Local<Object> JSObject::Wrap(Isolate *isolate, const ClassTypeInfo& type_info,
                             JSObject *base_ptr, void *ptr)
{
    return ClassRegistry::Get(isolate)->WrapObject(type_info, base_ptr, ptr);
}

void *JSObject::Unwrap(v8::Isolate *isolate, const ClassTypeInfo &type_info,
                       Local<Object> object)
{
    return ClassRegistry::Get(isolate)->UnwrapObject(type_info, object);
}

JSObject *JSObject::UnwrapBaseUnsafe(Isolate *isolate, Local<Object> object)
{
    return ClassRegistry::Get(isolate)->UnwrapObjectBase(object);
}

JSObject::JSObject()
        : class_metadata_(nullptr)
        , dispose_state_(DisposeState::kNot)
{
}

void JSObject::NotifyDisposeState(DisposeState state)
{
    dispose_state_ = state;
}

std::unique_ptr<JSTransferData>
JSObject::SerializeObject(Isolate *isolate, SerializeType type)
{
    if (type == SerializeType::kTransfer)
    {
        if (class_metadata_->attrs & ClassMetadata::kTransferable_Attr)
            return OnObjectTransfer(isolate);
    }
    else if (type == SerializeType::kClone)
    {
        if (class_metadata_->attrs & ClassMetadata::kCloneable_Attr)
            return OnObjectClone(isolate);
    }
    else
        MARK_UNREACHABLE();
    return nullptr;
}

std::unique_ptr<JSTransferData> JSObject::OnObjectTransfer(Isolate *isolate)
{
    MARK_UNREACHABLE("Not implemented");
}

std::unique_ptr<JSTransferData> JSObject::OnObjectClone(Isolate *isolate)
{
    MARK_UNREACHABLE("Not implemented");
}

GALLIUM_FFI_NS_END
