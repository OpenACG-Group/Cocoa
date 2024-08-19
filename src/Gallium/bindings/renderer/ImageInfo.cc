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

#include "Gallium/bindings/renderer/ImageInfo.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::Ret<int32_t> ColorTypeBytesPerPixel(ffi::Enum<SkColorType> ct)
{
    return SkColorTypeBytesPerPixel(*ct);
}

ffi::Ret<bool> ColorTypeIsAlwaysOpaque(ffi::Enum<SkColorType> ct)
{
    return SkColorTypeIsAlwaysOpaque(*ct);
}

ffi::RetLocal<v8::Object> ImageInfo::Make(int32_t width,
                                          int32_t height,
                                          ffi::Enum<SkColorType> color_type,
                                          ffi::Enum<SkAlphaType> alpha_type,
                                          ffi::Opt<ffi::Class<ColorSpace>> color_space)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, SkImageInfo::Make(
            width, height, *color_type, *alpha_type,
            color_space ? (*color_space)->GetColorSpace() : nullptr));
}

ffi::RetLocal<v8::Object> ImageInfo::MakeN32(int32_t width,
                                             int32_t height,
                                             ffi::Enum<SkAlphaType> alpha_type,
                                             ffi::Opt<ffi::Class<ColorSpace>> color_space)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, SkImageInfo::MakeN32(
            width, height, *alpha_type,
            color_space ? (*color_space)->GetColorSpace() : nullptr));
}

ffi::RetLocal<v8::Object> ImageInfo::MakeS32(int32_t width,
                                             int32_t height,
                                             ffi::Enum<SkAlphaType> alpha_type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, SkImageInfo::MakeS32(
            width, height, *alpha_type));
}

ffi::RetLocal<v8::Object> ImageInfo::MakeN32Premul(int32_t width,
                                                   int32_t height,
                                                   ffi::Opt<ffi::Class<ColorSpace>> color_space)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, SkImageInfo::MakeN32Premul(
            width, height,
            color_space ? (*color_space)->GetColorSpace() : nullptr));
}

ffi::RetLocal<v8::Object> ImageInfo::MakeA8(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, SkImageInfo::MakeA8(width, height));
}

ffi::RetLocal<v8::Object> ImageInfo::MakeUnknown(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, SkImageInfo::MakeUnknown(width, height));
}

ffi::RetLocal<v8::Value> ImageInfo::refColorSpace()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    sk_sp<SkColorSpace> cs = image_info_.refColorSpace();
    if (!cs)
        return v8::Null(isolate);
    return ffi::JSObject::New<ColorSpace>(isolate, cs);
}

ffi::RetLocal<v8::Object> ImageInfo::makeWH(int32_t width, int32_t height)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, image_info_.makeWH(width, height));
}

ffi::RetLocal<v8::Object> ImageInfo::makeAlphaType(ffi::Enum<SkAlphaType> alpha_type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, image_info_.makeAlphaType(*alpha_type));
}

ffi::RetLocal<v8::Object> ImageInfo::makeColorType(ffi::Enum<SkColorType> color_type)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, image_info_.makeColorType(*color_type));
}

ffi::RetLocal<v8::Object> ImageInfo::makeColorSpace(ffi::Opt<ffi::Class<ColorSpace>> cs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ImageInfo>(isolate, image_info_.makeColorSpace(
            cs ? (*cs)->GetColorSpace() : nullptr));
}

ffi::Ret<size_t> ImageInfo::computeOffset(int32_t x, int32_t y, size_t row_bytes)
{
    return image_info_.computeOffset(x, y, row_bytes);
}

ffi::Ret<bool> ImageInfo::equalsTo(ffi::Class<ImageInfo> other)
{
    return image_info_ == other->image_info_;
}

ffi::Ret<size_t> ImageInfo::computeByteSize(size_t row_bytes)
{
    return image_info_.computeByteSize(row_bytes);
}

ffi::Ret<size_t> ImageInfo::computeMinByteSize()
{
    return image_info_.computeMinByteSize();
}

ffi::Ret<bool> ImageInfo::validRowBytes(size_t row_bytes)
{
    return image_info_.validRowBytes(row_bytes);
}

ffi::Ret<void> ImageInfo::reset()
{
    image_info_.reset();
    return {};
}

GALLIUM_BINDINGS_RENDERER_NS_END
