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

#include "include/core/SkData.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkStream.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CanvasKitTransferContext.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkPictureWrap::CkPictureWrap(sk_sp<SkPicture> picture)
        : picture_(std::move(picture))
        , picture_size_hint_(0)
{
    CHECK(picture_);

    picture_size_hint_ = picture_->approximateBytesUsed();

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->AdjustAmountOfExternalAllocatedMemory(
            static_cast<int64_t>(picture_size_hint_));
}

CkPictureWrap::~CkPictureWrap()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (isolate)
    {
        isolate->AdjustAmountOfExternalAllocatedMemory(
                -static_cast<int64_t>(picture_size_hint_));
    }
}

v8::Local<v8::Value> CkPictureWrap::serialize()
{
    sk_sp<SkData> data = picture_->serialize();
    if (data == nullptr)
        g_throw(Error, "Failed to serialize SkPicture object");

    // Maybe there is a way to avoid this expensive memory copy
    v8::Local<v8::Object> buffer = Buffer::MakeFromPtrCopy(data->data(), data->size());
    return buffer;
}

const sk_sp<SkPicture>& CkPictureWrap::getPicture() const
{
    return picture_;
}

uint32_t CkPictureWrap::approximateOpCount(bool nested)
{
    return picture_->approximateOpCount(nested);
}

uint32_t CkPictureWrap::approximateByteUsed()
{
    return picture_->approximateBytesUsed();
}

namespace {

sk_sp<SkTypeface>
deserialize_typeface_in_transfer_mode(const void* data, size_t length, void *ctx)
{
    CHECK(gl::GlobalScope::Instance());

    // It is very wired that Skia provides a SkStream** pointer as a
    // `const void *data` argument. One possible reason may be that all the
    // types of SkStream does NOT expose a directly accessible memory address
    // to the contained data. Argument `length` is actually `sizeof(SkStream*)`,
    // which we do not need.
    CHECK(length == sizeof(uintptr_t));
    SkStream *stream = *reinterpret_cast<SkStream**>(const_cast<void*>(data));

    auto parsed_key = CanvasKitTransferContext::TypefaceKey::ParseFromBinary(stream);
    if (!parsed_key)
        return nullptr;

    auto *transfer_context = reinterpret_cast<CanvasKitTransferContext*>(
            gl::GlobalScope::Ref().GetExternalDataPointer());
    CHECK(transfer_context);

    return transfer_context->RequestTypeface(*parsed_key);
}

} // namespace anonymous

v8::Local<v8::Value> CkPictureWrap::MakeFromData(v8::Local<v8::Value> bufferObject,
                                                 PictureUsage usage)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Buffer *buffer = binder::Class<Buffer>::unwrap_object(isolate, bufferObject);
    if (!buffer)
        g_throw(TypeError, "'buffer' must be an instance of core.Buffer");

    SkDeserialProcs deserial_procs{};
    if (usage == kTransfer_Usage)
        deserial_procs.fTypefaceProc = deserialize_typeface_in_transfer_mode;

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(buffer->addressU8(), buffer->length(),
                                                    &deserial_procs);
    if (!pict)
        g_throw(Error, "Cannot serialize a CkPicture from buffer");

    return binder::Class<CkPictureWrap>::create_object(isolate, pict);
}

v8::Local<v8::Value> CkPictureWrap::MakeFromFile(const std::string& path,
                                                 PictureUsage usage)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
    if (!data)
        g_throw(Error, fmt::format("Failed to read file {}", path));

    SkDeserialProcs deserial_procs{};
    if (usage == kTransfer_Usage)
        deserial_procs.fTypefaceProc = deserialize_typeface_in_transfer_mode;

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get(), &deserial_procs);
    if (!pict)
        g_throw(Error, fmt::format("Cannot serialize a CkPicture from {}", path));

    return binder::Class<CkPictureWrap>::create_object(v8::Isolate::GetCurrent(), pict);
}

uint32_t CkPictureWrap::uniqueId()
{
    return picture_->uniqueID();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
