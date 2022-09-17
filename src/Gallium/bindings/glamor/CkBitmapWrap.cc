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

#include <utility>

#include "include/core/SkImageInfo.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/codec/SkCodec.h"

#include "fmt/format.h"

#include "Gallium/bindings/core/Exports.h"
#include "Gallium/bindings/glamor/Exports.h"

GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkBitmapWrap::CkBitmapWrap(v8::Isolate *isolate,
                           v8::Local<v8::Object> buffer_object,
                           std::shared_ptr<SkBitmap> bitmap)
    : buffer_object_(isolate, buffer_object)
    , bitmap_(std::move(bitmap))
{
    CHECK(bitmap_ && "CkBitmap was constructed with an invalid bitmap");
}

namespace {

v8::Local<v8::Value> create_bitmap_from_buffer(v8::Local<v8::Object> buffer,
                                               Buffer *unwrapped_buffer,
                                               const SkImageInfo& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    size_t pixelsSize = info.computeMinByteSize();

    // Do a quick check based on the size of the provided buffer.
    if (pixelsSize != unwrapped_buffer->length())
        g_throw(Error, "Provided buffer size conflicts with the image info");

    // Bitmap shares the same memory with `buffer`, and keeps the ownership of `buffer`
    auto bitmap = std::make_shared<SkBitmap>();
    bitmap->installPixels(info, unwrapped_buffer->addressU8(),
                          info.minRowBytes());

    return binder::Class<CkBitmapWrap>::create_object(isolate, isolate, buffer, bitmap);
}

} // namespace anonymous

v8::Local<v8::Value> CkBitmapWrap::MakeFromBuffer(v8::Local<v8::Value> buffer,
                                                  int32_t width,
                                                  int32_t height,
                                                  uint32_t colorType,
                                                  uint32_t alphaType)
{
    // Check arguments provided by user. Enumeration value that is out of range
    // may cause an unexpected and serious result.
    if (width <= 0 || height <= 0)
        g_throw(Error, fmt::format("Invalid geometry size of bitmap ({}x{})", width, height));

    if (colorType > SkColorType::kLastEnum_SkColorType)
        g_throw(Error, "Invalid enumeration value of colorType");
    if (alphaType > SkAlphaType::kLastEnum_SkAlphaType)
        g_throw(Error, "Invalid enumeration value of alphaType");

    // Construct a bitmap and fill it with data in `buffer`

    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    Buffer *unwrapped = binder::Class<Buffer>::unwrap_object(isolate, buffer);
    if (!unwrapped)
        g_throw(TypeError, "'buffer' must be an instance of core.Buffer");

    SkImageInfo imageInfo = SkImageInfo::Make(width, height,
                                              static_cast<SkColorType>(colorType),
                                              static_cast<SkAlphaType>(alphaType));

    return create_bitmap_from_buffer(v8::Local<v8::Object>::Cast(buffer),
                                     unwrapped, imageInfo);
}

v8::Local<v8::Value> CkBitmapWrap::MakeFromEncodedFile(const std::string& path)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    std::unique_ptr<SkStream> stream = SkStream::MakeFromFile(path.c_str());
    if (!stream)
        g_throw(Error, fmt::format("Failed to read file {} to decode bitmap", path));

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream), &result);
    if (!codec)
    {
        g_throw(Error, fmt::format("Failed to decode file {}: {}",
                                   path, SkCodec::ResultToString(result)));
    }

    SkImageInfo info = codec->getInfo();

    v8::Local<v8::Value> buffer = Buffer::MakeFromSize(info.computeMinByteSize());
    Buffer *unwrapped = binder::Class<Buffer>::unwrap_object(isolate, buffer);
    CHECK(unwrapped);

    result = codec->getPixels(info, unwrapped->addressU8(),
                              info.minRowBytes());
    if (result != SkCodec::Result::kSuccess)
    {
        g_throw(Error, fmt::format("Failed to read pixels from file {}: {}",
                                   path, SkCodec::ResultToString(result)));
    }

    return create_bitmap_from_buffer(v8::Local<v8::Object>::Cast(buffer),
                                     unwrapped, info);
}

int32_t CkBitmapWrap::getWidth()
{
    return bitmap_->width();
}

int32_t CkBitmapWrap::getHeight()
{
    return bitmap_->height();
}

uint32_t CkBitmapWrap::getAlphaType()
{
    return static_cast<uint32_t>(bitmap_->alphaType());
}

uint32_t CkBitmapWrap::getColorType()
{
    return static_cast<uint32_t>(bitmap_->colorType());
}

int32_t CkBitmapWrap::getBytesPerPixel()
{
    return bitmap_->bytesPerPixel();
}

int32_t CkBitmapWrap::getRowBytesAsPixels()
{
    return bitmap_->rowBytesAsPixels();
}

int32_t CkBitmapWrap::getShiftPerPixel()
{
    return bitmap_->shiftPerPixel();
}

size_t CkBitmapWrap::getRowBytes()
{
    return bitmap_->rowBytes();
}

size_t CkBitmapWrap::computeByteSize()
{
    return bitmap_->computeByteSize();
}

v8::Local<v8::Value> CkBitmapWrap::toImage()
{
    sk_sp<SkImage> image = bitmap_->asImage();
    if (!image)
        g_throw(Error, "Cannot convert the bitmap to a CkImage");
    return binder::Class<CkImageWrap>::create_object(v8::Isolate::GetCurrent(), image);
}

v8::Local<v8::Value> CkBitmapWrap::getPixelBuffer()
{
    return buffer_object_.Get(v8::Isolate::GetCurrent());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
