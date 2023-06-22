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

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/CkMatrixWrap.h"

GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkBitmapWrap::CkBitmapWrap(v8::Isolate *isolate,
                           v8::Local<v8::ArrayBuffer> buffer_object,
                           std::shared_ptr<SkBitmap> bitmap)
    : array_buffer_(isolate, buffer_object)
    , bitmap_(std::move(bitmap))
{
    CHECK(bitmap_ && "CkBitmap was constructed with an invalid bitmap");
}

namespace {

v8::Local<v8::Value> create_bitmap_from_buffer(v8::Local<v8::ArrayBuffer> buffer,
                                               const SkImageInfo& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    size_t pixelsSize = info.computeMinByteSize();

    // Do a quick check based on the size of the provided buffer
    if (pixelsSize != buffer->ByteLength())
        g_throw(Error, "Provided buffer size conflicts with the image info");

    // Bitmap shares the same memory with `buffer`, and keeps the ownership of `buffer`
    auto bitmap = std::make_shared<SkBitmap>();
    bitmap->installPixels(info, buffer->Data(), info.minRowBytes());

    return binder::NewObject<CkBitmapWrap>(isolate, isolate, buffer, bitmap);
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

    SkImageInfo imageInfo = SkImageInfo::Make(width, height,
                                              static_cast<SkColorType>(colorType),
                                              static_cast<SkAlphaType>(alphaType));

    if (!buffer->IsArrayBuffer())
        g_throw(TypeError, "Argument `buffer` must be an ArrayBuffer");

    return create_bitmap_from_buffer(buffer.As<v8::ArrayBuffer>(), imageInfo);
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

    v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(isolate, info.computeMinByteSize());

    result = codec->getPixels(info, buffer->Data(), info.minRowBytes());
    if (result != SkCodec::Result::kSuccess)
    {
        g_throw(Error, fmt::format("Failed to read pixels from file {}: {}",
                                   path, SkCodec::ResultToString(result)));
    }

    return create_bitmap_from_buffer(buffer, info);
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
    return binder::NewObject<CkImageWrap>(v8::Isolate::GetCurrent(), image);
}

v8::Local<v8::Value> CkBitmapWrap::makeShader(int32_t tmx, int32_t tmy, int32_t sampling,
                                              v8::Local<v8::Value> local_matrix)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (tmx < 0 || tmx > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "Invalid enumeration value for argument `tmx`");
    if (tmy < 0 || tmy > static_cast<int32_t>(SkTileMode::kLastTileMode))
        g_throw(RangeError, "Invalid enumeration value for argument `tmy`");

    SkMatrix *matrix = nullptr;
    if (!local_matrix->IsNullOrUndefined())
    {
        auto *w = binder::UnwrapObject<CkMatrix>(isolate, local_matrix);
        if (!w)
            g_throw(TypeError, "Argument `localMatrix` requires an instance of `CkMatrix` or null");
        matrix = &w->GetMatrix();
    }

    sk_sp<SkShader> shader = bitmap_->makeShader(static_cast<SkTileMode>(tmx),
                                                 static_cast<SkTileMode>(tmy),
                                                 SamplingToSamplingOptions(sampling),
                                                 matrix);
    if (!shader)
        g_throw(Error, "Failed to create shader from bitmap");

    return binder::NewObject<CkShaderWrap>(isolate, shader);
}

v8::Local<v8::Value> CkBitmapWrap::getPixelBuffer()
{
    return array_buffer_.Get(v8::Isolate::GetCurrent());
}

GALLIUM_BINDINGS_GLAMOR_NS_END
