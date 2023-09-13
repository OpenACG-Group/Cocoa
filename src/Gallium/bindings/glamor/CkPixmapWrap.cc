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

#include "Gallium/binder/ThrowExcept.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/bindings/glamor/CkPixmapWrap.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

CkPixmap::CkPixmap(const v8::FunctionCallbackInfo<v8::Value>& call_info)
{
    if (call_info.Length() == 0)
        return;

    if (call_info.Length() != 3)
        g_throw(Error, "Invalid number of arguments, expecting 0 or 3 arguments");

    if (!call_info[1]->IsUint32())
        g_throw(TypeError, "Argument `rowBytes` must be a u32 number");
    size_t row_bytes = call_info[1].As<v8::Uint32>()->Value();
    CkPixmap::reset(call_info[0], static_cast<int64_t>(row_bytes), call_info[2]);
}

CkPixmap::CkPixmap(SkPixmap pixmap)
    : pixmap_(std::move(pixmap))
{
}

void CkPixmap::CheckEmptyOrThrow()
{
    if (!pixmap_.addr())
        g_throw(Error, "Empty pixmap");
}

void CkPixmap::resetEmpty()
{
    pixmap_.reset();
}

void CkPixmap::reset(v8::Local<v8::Value> image_info, int64_t row_bytes, v8::Local<v8::Value> buffer)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkImageInfo sk_image_info = ExtractCkImageInfo(isolate, image_info);
    auto memory = binder::GetTypedArrayMemory<v8::TypedArray>(buffer);
    if (!memory)
        g_throw(TypeError, "Argument `buffer` must be a valid TypedArray");
    if (memory->byte_size < sk_image_info.computeByteSize(row_bytes))
        g_throw(Error, "Buffer is not big enough to contain the pixels");
    pixmap_.reset(sk_image_info, memory->ptr, row_bytes);
}

v8::Local<v8::Value> CkPixmap::extractSubset(v8::Local<v8::Value> area)
{
    CheckEmptyOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkPixmap subset;
    if (!pixmap_.extractSubset(&subset, ExtractCkRect(isolate, area).round()))
        return v8::Null(isolate);
    return binder::NewObject<CkPixmap>(isolate, std::move(subset));
}

v8::Local<v8::Value> CkPixmap::getInfo()
{
    CheckEmptyOrThrow();
    return NewCkImageInfo(v8::Isolate::GetCurrent(), pixmap_.info());
}

v8::Local<v8::Value> CkPixmap::getRowBytes()
{
    CheckEmptyOrThrow();
    return binder::to_v8(v8::Isolate::GetCurrent(), pixmap_.rowBytes());
}

int32_t CkPixmap::getWidth()
{
    CheckEmptyOrThrow();
    return pixmap_.width();
}

int32_t CkPixmap::getHeight()
{
    CheckEmptyOrThrow();
    return pixmap_.height();
}

int32_t CkPixmap::getColorType()
{
    CheckEmptyOrThrow();
    return pixmap_.colorType();
}

int32_t CkPixmap::getAlphaType()
{
    CheckEmptyOrThrow();
    return pixmap_.alphaType();
}

bool CkPixmap::getIsOpaque()
{
    CheckEmptyOrThrow();
    return pixmap_.isOpaque();
}

v8::Local<v8::Value> CkPixmap::getBounds()
{
    CheckEmptyOrThrow();
    return NewCkRect(v8::Isolate::GetCurrent(),
                     SkRect::Make(pixmap_.bounds()));
}

int32_t CkPixmap::getRowBytesAsPixels()
{
    CheckEmptyOrThrow();
    return pixmap_.rowBytesAsPixels();
}

int32_t CkPixmap::getShiftPerPixel()
{
    CheckEmptyOrThrow();
    return pixmap_.shiftPerPixel();
}

int64_t CkPixmap::computeByteSize()
{
    CheckEmptyOrThrow();
    size_t byte_size = pixmap_.computeByteSize();
    CHECK(byte_size <= INT64_MAX);
    return static_cast<int64_t>(byte_size);
}

bool CkPixmap::computeIsOpaque()
{
    CheckEmptyOrThrow();
    return pixmap_.computeIsOpaque();
}

#define CHECK_POS_RANGE(x, y) \
    if (x < 0 || x >= pixmap_.width() || y < 0 || y >= pixmap_.height()) \
        g_throw(RangeError, "Position is out of the pixmap");            \

v8::Local<v8::Value> CkPixmap::getColor4f(int32_t x, int32_t y)
{
    CheckEmptyOrThrow();
    CHECK_POS_RANGE(x, y)
    return NewColor4f(v8::Isolate::GetCurrent(), pixmap_.getColor4f(x, y));
}

float CkPixmap::getAlphaf(int32_t x, int32_t y)
{
    CheckEmptyOrThrow();
    CHECK_POS_RANGE(x, y)
    return pixmap_.getAlphaf(x, y);
}

void CkPixmap::readPixels(v8::Local<v8::Value> dst_info,
                          v8::Local<v8::Value> dst_buffer,
                          int64_t dst_row_bytes,
                          int32_t src_x,
                          int32_t src_y)
{
    CheckEmptyOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_POS_RANGE(src_x, src_y)
    auto *info_wrap = binder::UnwrapObject<CkImageInfo>(isolate, dst_info);
    if (!info_wrap)
        g_throw(TypeError, "Argument `dstInfo` must be an instance of `CkImageInfo`");
    if (info_wrap->GetWrapped().minRowBytes() > dst_row_bytes)
        g_throw(Error, "`dstRowBytes` is too small to contain one row of pixels");

    auto dst_mem = binder::GetTypedArrayMemory<v8::TypedArray>(dst_buffer);
    if (!dst_mem)
        g_throw(TypeError, "Argument `dstBuffer` must be an allocated TypedArray");

    if (!pixmap_.readPixels(info_wrap->GetWrapped(), dst_mem->ptr, dst_row_bytes, src_x, src_y))
        g_throw(Error, "Failed to read pixels");
}

void CkPixmap::copy(v8::Local<v8::Value> dst, int32_t src_x, int32_t src_y)
{
    CheckEmptyOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_POS_RANGE(src_x, src_y)
    CkPixmap *dst_pixmap = binder::UnwrapObject<CkPixmap>(isolate, dst);
    if (!dst_pixmap || !dst_pixmap->GetInnerPixmap().addr())
        g_throw(TypeError, "Argument `dst` must be a non-empty CkPixmap");
    if (!pixmap_.readPixels(dst_pixmap->GetInnerPixmap(), src_x, src_y))
        g_throw(Error, "Failed to read pixels from pixmap: maybe format conversion is impossible");
}

void CkPixmap::scale(v8::Local<v8::Value> dst, int32_t sampling)
{
    CheckEmptyOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CkPixmap *dst_pixmap = binder::UnwrapObject<CkPixmap>(isolate, dst);
    if (!dst_pixmap || !dst_pixmap->GetInnerPixmap().addr())
        g_throw(TypeError, "Argument `dst` must be a non-empty CkPixmap");
    SkSamplingOptions sampling_opt = SamplingToSamplingOptions(sampling);
    if (!pixmap_.scalePixels(dst_pixmap->GetInnerPixmap(), sampling_opt))
        g_throw(Error, "Failed to read and scale pixels: maybe format conversion is impossible");
}

void CkPixmap::erase(v8::Local<v8::Value> color, v8::Local<v8::Value> subset)
{
    CheckEmptyOrThrow();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkColor4f color4f = ExtractColor4f(isolate, color);
    SkIRect *subset_rect_ptr = nullptr;
    SkIRect subset_rect;
    if (!subset->IsNullOrUndefined())
    {
        subset_rect = ExtractCkRect(isolate, subset).round();
        subset_rect_ptr = &subset_rect;
    }
    if (!pixmap_.erase(color4f, subset_rect_ptr))
        g_throw(Error, "Failed to erase pixels in pixmap: unknown color type or invalid `subset`");
}

GALLIUM_BINDINGS_GLAMOR_NS_END
