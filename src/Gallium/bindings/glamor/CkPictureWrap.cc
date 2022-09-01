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

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/core/Exports.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkPictureWrap::CkPictureWrap(sk_sp<SkPicture> picture)
        : picture_(std::move(picture))
{
    CHECK(picture_);
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

v8::Local<v8::Value> CkPictureWrap::MakeFromData(v8::Local<v8::Value> bufferObject)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    Buffer *buffer = binder::Class<Buffer>::unwrap_object(isolate, bufferObject);
    if (!buffer)
        g_throw(TypeError, "'buffer' must be an instance of core.Buffer");

    sk_sp<SkData> data = SkData::MakeWithoutCopy(buffer->addressU8(),
                                                 buffer->length());
    CHECK(data);

    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get());
    if (!pict)
        g_throw(Error, "Cannot serialize a CkPicture from buffer");

    return binder::Class<CkPictureWrap>::create_object(isolate, pict);
}

v8::Local<v8::Value> CkPictureWrap::MakeFromFile(const std::string& path)
{
    sk_sp<SkData> data = SkData::MakeFromFileName(path.c_str());
    sk_sp<SkPicture> pict = SkPicture::MakeFromData(data.get());
    if (!pict)
        g_throw(Error, fmt::format("Cannot serialize a CkPicture from {}", path));

    return binder::Class<CkPictureWrap>::create_object(v8::Isolate::GetCurrent(), pict);
}

uint32_t CkPictureWrap::uniqueId()
{
    return picture_->uniqueID();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
