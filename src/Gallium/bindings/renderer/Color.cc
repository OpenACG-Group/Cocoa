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

#include "Gallium/bindings/renderer/Color.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

ffi::RetLocal<v8::Object> ColorSpace::MakeSRGB()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorSpace>(isolate, SkColorSpace::MakeSRGB());
}

ffi::RetLocal<v8::Object> ColorSpace::MakeSRGBLinear()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorSpace>(isolate, SkColorSpace::MakeSRGBLinear());
}

ffi::Ret<bool> ColorSpace::Equals(ffi::Class<ColorSpace> cs1, ffi::Class<ColorSpace> cs2)
{
    return SkColorSpace::Equals(cs1->color_space_.get(),
                                cs2->color_space_.get());
}

ffi::Ret<void> ColorSpace::dispose()
{
    color_space_.reset();
    NotifyDisposeState(DisposeState::kDisposed);
    return {};
}

ffi::RetLocal<v8::Object> ColorSpace::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorSpace>(isolate, color_space_);
}

ffi::Ret<bool> ColorSpace::gammaCloseToSRGB()
{
    return color_space_->gammaCloseToSRGB();
}

ffi::Ret<bool> ColorSpace::gammaIsLinear()
{
    return color_space_->gammaIsLinear();
}

ffi::RetLocal<v8::Object> ColorSpace::makeLinearGamma()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorSpace>(isolate, color_space_->makeLinearGamma());
}

ffi::RetLocal<v8::Object> ColorSpace::makeSRGBGamma()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorSpace>(isolate, color_space_->makeSRGBGamma());
}

ffi::RetLocal<v8::Object> ColorSpace::makeColorSpin()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorSpace>(isolate, color_space_->makeColorSpin());
}

ffi::Ret<bool> ColorSpace::isSRGB()
{
    return color_space_->isSRGB();
}

ffi::RetLocal<v8::Value> ColorMatrix::RGBtoYUV(const ffi::Enum<SkYUVColorSpace>& cs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorMatrix>(isolate, SkColorMatrix::RGBtoYUV(*cs));
}

ffi::RetLocal<v8::Value> ColorMatrix::YUVtoRGB(const ffi::Enum<SkYUVColorSpace>& cs)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<ColorMatrix>(isolate, SkColorMatrix::YUVtoRGB(*cs));
}

ffi::Ret<void> ColorMatrix::setIdentity()
{
    mat_.setIdentity();
    return {};
}

ffi::Ret<void> ColorMatrix::setScale(float sr, float sg, float sb, float sa)
{
    mat_.setScale(sr, sg, sb, sa);
    return {};
}

ffi::Ret<void> ColorMatrix::postTranslate(float dr, float dg, float db, float da)
{
    mat_.postTranslate(dr, dg, db, da);
    return {};
}

ffi::Ret<void> ColorMatrix::setConcat(const ffi::Class<ColorMatrix>& a,
                                      const ffi::Class<ColorMatrix>& b)
{
    mat_.setConcat(a->mat_, b->mat_);
    return {};
}

ffi::Ret<void> ColorMatrix::preConcat(const ffi::Class<ColorMatrix>& mat)
{
    mat_.preConcat(mat->mat_);
    return {};
}

ffi::Ret<void> ColorMatrix::postConcat(const ffi::Class<ColorMatrix>& mat)
{
    mat_.postConcat(mat->mat_);
    return {};
}

ffi::Ret<void> ColorMatrix::setSaturation(float sat)
{
    mat_.setSaturation(sat);
    return {};
}

ffi::Ret<void> ColorMatrix::setRowMajor(const ffi::Mem<float>& src)
{
    if (src.Size() < 20)
        return ffi::Fail(ffi::kErr, "size of row-major array is insufficient (less than 20)");
    mat_.setRowMajor(src.Address());
    return {};
}

ffi::Ret<void> ColorMatrix::getRowMajor(const ffi::Mem<float>& dst)
{
    if (dst.Size() < 20)
        return ffi::Fail(ffi::kErr, "size of row-major array is insufficient (less than 20)");
    mat_.getRowMajor(dst.Address());
    return {};
}

GALLIUM_BINDINGS_RENDERER_NS_END
