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

#include "Gallium/bindings/renderer/Pixmap.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::Ret<PixmapAdapter> PixmapAdapter::Cast(v8::Isolate *isolate, v8::Local<v8::Value> value)
{
    auto result = ffi::Cast<ffi::IFace<Pixmap>>::From(isolate, value);
    if (result.HasError())
        return result.GetError();

    ffi::IFace<Pixmap>& pixmap = result.Extract();
    size_t size = pixmap->image_info->GetImageInfo().computeByteSize(pixmap->row_bytes);
    if (pixmap->pixels.ByteSize() < size)
        return ffi::Fail(ffi::kErr, "size of buffer does not match the image info");

    return PixmapAdapter{
        .memory = pixmap->pixels,
        .pixmap = pixmap->ToSkPixmap()
    };
}

ffi::Ret<bool> PixmapUtils::ComputeIsOpaque(PixmapAdapter src)
{
    return src->computeIsOpaque();
}

ffi::Ret<bool> PixmapUtils::Copy(PixmapAdapter src, PixmapAdapter dst,
                                 int32_t src_x, int32_t src_y)
{
    return src->readPixels(*dst, src_x, src_y);
}

ffi::Ret<bool> PixmapUtils::EraseSubset(PixmapAdapter dst, uint32_t color,
                                        RectAdapter subset)
{
    return dst->erase(color, (*subset).round());
}

ffi::Ret<bool> PixmapUtils::Erase(PixmapAdapter dst, uint32_t color)
{
    return dst->erase(color);
}

ffi::Ret<bool> PixmapUtils::Scale(PixmapAdapter src, PixmapAdapter dst,
                                  SamplingOptionsAdapter sampling)
{
    return src->scalePixels(*dst, *sampling);
}

ffi::Ret<uint32_t> PixmapUtils::PickColor(PixmapAdapter src, int32_t x, int32_t y)
{
    const SkImageInfo& info = src->info();
    if (x < 0 || x >= info.width() || y < 0 || y >= info.height())
        return ffi::Fail(ffi::kRangeErr, "(x, y) position is out of bounds");

    return src->getColor(x, y);
}

GALLIUM_BINDINGS_RENDERER_NS_END
