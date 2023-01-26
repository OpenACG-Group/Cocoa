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

#include "Gallium/bindings/glamor/CkMatrixWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define EXTRACT_MAT_CHECKED(arg, result) \
    auto *result = binder::Class<CkMatrix>::unwrap_object(isolate, arg); \
    if (!result) {                       \
        g_throw(TypeError, "Argument `" #arg "` must be an instance of `CkPath`"); \
    }

#define CHECK_ENUM_RANGE(v, last) \
    if (v < 0 || v > static_cast<int32_t>(last)) { \
        g_throw(RangeError, "Invalid enumeration value for arguemnt `" #v "`"); \
    }

using CLASS = binder::Class<CkMatrix>;

v8::Local<v8::Value> CkMatrix::Identity()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, SkMatrix::I());
}

v8::Local<v8::Value> CkMatrix::Scale(SkScalar sx, SkScalar sy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, SkMatrix::Scale(sx, sy));
}

v8::Local<v8::Value> CkMatrix::Translate(SkScalar dx, SkScalar dy)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, SkMatrix::Translate(dx, dy));
}

v8::Local<v8::Value> CkMatrix::RotateRad(SkScalar rad, v8::Local<v8::Value> pt)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, SkMatrix::RotateDeg(SkRadiansToDegrees(rad),
                                                             ExtractCkPoint(isolate, pt)));
}

v8::Local<v8::Value> CkMatrix::Skew(SkScalar kx, SkScalar ky)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, SkMatrix::Skew(kx, ky));
}

v8::Local<v8::Value> CkMatrix::RectToRect(v8::Local<v8::Value> src,
                                          v8::Local<v8::Value> dst, int32_t mode)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(mode, SkMatrix::kEnd_ScaleToFit)
    return CLASS::create_object(isolate, SkMatrix::MakeRectToRect(ExtractCkRect(isolate, src),
                                                                  ExtractCkRect(isolate, dst),
                                                                  static_cast<SkMatrix::ScaleToFit>(mode)));
}

v8::Local<v8::Value> CkMatrix::All(SkScalar scaleX,
                                   SkScalar skewX,
                                   SkScalar transX,
                                   SkScalar skewY,
                                   SkScalar scaleY,
                                   SkScalar transY,
                                   SkScalar pers0,
                                   SkScalar pers1,
                                   SkScalar pers2)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, SkMatrix::MakeAll(scaleX, skewX, transX,
                                                           skewY, scaleY, transY,
                                                           pers0, pers1, pers2));
}

v8::Local<v8::Value> CkMatrix::Concat(v8::Local<v8::Value> a, v8::Local<v8::Value> b)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_MAT_CHECKED(a, ma)
    EXTRACT_MAT_CHECKED(b, mb)
    return CLASS::create_object(isolate, SkMatrix::Concat(ma->matrix_, mb->matrix_));
}

#define IMPL_PROP_SETTER_GETTER(prop) \
    SkScalar CkMatrix::get##prop() { return matrix_.get##prop(); } \
    void CkMatrix::set##prop(SkScalar v) { matrix_.set##prop(v); }

#define IMPL_PROP_SETTER_GETTER2(prop, matProp) \
    SkScalar CkMatrix::get##prop() { return matrix_.get##matProp(); } \
    void CkMatrix::set##prop(SkScalar v) { matrix_.set##matProp(v); }

IMPL_PROP_SETTER_GETTER(ScaleX)
IMPL_PROP_SETTER_GETTER(ScaleY)
IMPL_PROP_SETTER_GETTER(SkewX)
IMPL_PROP_SETTER_GETTER(SkewY)
IMPL_PROP_SETTER_GETTER(TranslateX)
IMPL_PROP_SETTER_GETTER(TranslateY)
IMPL_PROP_SETTER_GETTER2(PerspectiveX, PerspX)
IMPL_PROP_SETTER_GETTER2(PerspectiveY, PerspY)

v8::Local<v8::Value> CkMatrix::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CLASS::create_object(isolate, matrix_);
}

bool CkMatrix::rectStaysRect()
{
    return matrix_.rectStaysRect();
}

bool CkMatrix::hasPerspective()
{
    return matrix_.hasPerspective();
}

bool CkMatrix::isSimilarity()
{
    return matrix_.isSimilarity();
}

bool CkMatrix::preservesRightAngles()
{
    return matrix_.preservesRightAngles();
}

void CkMatrix::preTranslate(SkScalar dx, SkScalar dy)
{
    matrix_.preTranslate(dx, dy);
}

void CkMatrix::preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    matrix_.preScale(sx, sy, px, py);
}

void CkMatrix::preRotate(SkScalar rad, SkScalar px, SkScalar py)
{
    matrix_.preRotate(SkRadiansToDegrees(rad), px, py);
}

void CkMatrix::preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
{
    matrix_.preSkew(kx, ky, px, py);
}

void CkMatrix::preConcat(v8::Local<v8::Value> other)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_MAT_CHECKED(other, m)
    matrix_.preConcat(m->matrix_);
}

void CkMatrix::postTranslate(SkScalar dx, SkScalar dy)
{
    matrix_.postTranslate(dx, dy);
}

void CkMatrix::postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
{
    matrix_.postScale(sx, sy, px, py);
}

void CkMatrix::postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
{
    matrix_.postSkew(kx, ky, px, py);
}

void CkMatrix::postRotate(SkScalar rad, SkScalar px, SkScalar py)
{
    matrix_.postRotate(SkRadiansToDegrees(rad), px, py);
}

void CkMatrix::postConcat(v8::Local<v8::Value> other)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    EXTRACT_MAT_CHECKED(other, m)
    matrix_.postConcat(m->matrix_);
}

v8::Local<v8::Value> CkMatrix::invert()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkMatrix mat;
    if (!matrix_.invert(&mat))
        return v8::Null(isolate);
    return CLASS::create_object(isolate, mat);
}

void CkMatrix::normalizePerspective()
{
    matrix_.normalizePerspective();
}

v8::Local<v8::Value> CkMatrix::mapPoints(v8::Local<v8::Value> points)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!points->IsArray())
        g_throw(TypeError, "Argument `points` must be an array of `CkPoint`");

    auto arr = v8::Local<v8::Array>::Cast(points);
    if (arr->Length() == 0)
        return v8::Array::New(isolate, 0);

    auto nb_points = static_cast<int32_t>(arr->Length());

    std::vector<SkPoint> srcpts(nb_points);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (int32_t i = 0; i < nb_points; i++)
    {
        auto v = arr->Get(ctx, i).ToLocalChecked();
        srcpts[i] = ExtractCkPoint(isolate, v);
    }

    std::vector<SkPoint> dstpts(nb_points);
    matrix_.mapPoints(dstpts.data(), srcpts.data(), nb_points);

    auto dstarr = v8::Array::New(isolate, nb_points);
    for (int32_t i = 0; i < nb_points; i++)
        dstarr->Set(ctx, i, WrapCkPoint(isolate, dstpts[i])).Check();

    return dstarr;
}

v8::Local<v8::Value> CkMatrix::mapPoint(v8::Local<v8::Value> point)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return WrapCkPoint(isolate, matrix_.mapPoint(ExtractCkPoint(isolate, point)));
}

v8::Local<v8::Value> CkMatrix::mapHomogeneousPoints(v8::Local<v8::Value> points)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!points->IsArray())
        g_throw(TypeError, "Argument `points` must be an array of `CkPoint`");

    auto arr = v8::Local<v8::Array>::Cast(points);
    if (arr->Length() == 0)
        return v8::Array::New(isolate, 0);

    auto nb_points = static_cast<int32_t>(arr->Length());

    std::vector<SkPoint3> srcpts(nb_points);

    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (int32_t i = 0; i < nb_points; i++)
    {
        auto v = arr->Get(ctx, i).ToLocalChecked();
        srcpts[i] = ExtractCkPoint3(isolate, v);
    }

    std::vector<SkPoint3> dstpts(nb_points);
    matrix_.mapHomogeneousPoints(dstpts.data(), srcpts.data(), nb_points);

    auto dstarr = v8::Array::New(isolate, nb_points);
    for (int32_t i = 0; i < nb_points; i++)
        dstarr->Set(ctx, i, WrapCkPoint3(isolate, dstpts[i])).Check();

    return dstarr;
}

SkScalar CkMatrix::mapRadius(SkScalar radius)
{
    return matrix_.mapRadius(radius);
}

v8::Local<v8::Value> CkMatrix::mapRect(v8::Local<v8::Value> src, int32_t pc)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_ENUM_RANGE(pc, SkApplyPerspectiveClip::kYes)
    return WrapCkRect(isolate, matrix_.mapRect(ExtractCkRect(isolate, src),
                                               static_cast<SkApplyPerspectiveClip>(pc)));
}

bool CkMatrix::isFinite()
{
    return matrix_.isFinite();
}

GALLIUM_BINDINGS_GLAMOR_NS_END
