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
#include "include/core/SkPathUtils.h"

#include "Gallium/bindings/renderer/Path.h"
#include "Gallium/bindings/renderer/Paint.h"
GALLIUM_BINDINGS_RENDERER_NS_BEGIN

namespace {

class CloneData : public ffi::JSTransferData
{
public:
    explicit CloneData(const SkPath& path) : path_(path) {}
    ~CloneData() override = default;

    ffi::RetLocal<v8::Object>
    Construct(v8::Isolate *isolate, v8::Local<v8::Context> ctx) override
    {
        // Copy `SkPath` by value is efficient as the underlying data pointer
        // is shared among different copies. Memory allocation will not occur
        // until the path is modified.
        return ffi::JSObject::New<Path>(isolate, path_);
    }

private:
    SkPath path_;
};

} // namespace anonymous

std::unique_ptr<ffi::JSTransferData> Path::OnObjectClone(v8::Isolate *isolate)
{
    return std::make_unique<CloneData>(path_);
}

ffi::RetLocal<v8::Value> Path::Make(const ffi::Mem<float>& points,
                                    const ffi::Mem<uint8_t>& verbs,
                                    const ffi::Mem<float>& weights,
                                    const ffi::Enum<SkPathFillType>& fill_type,
                                    bool is_volatile)
{
    // To make sure the reinterpret_cast that we use later is valid.
    static_assert(std::is_standard_layout_v<SkPoint> && std::is_trivial_v<SkPoint> &&
                  sizeof(SkPoint) == sizeof(float) * 2);

    if (points.Size() & 1)
        return ffi::Fail(ffi::kErr, "invalid length of array as flattened points");

    SkPath path = SkPath::Make(
        // Considering the memory layout of `SkPoint` struct, this is equivalent
        // to create an array of `SkPoint`
        reinterpret_cast<SkPoint*>(points.Address()),
        static_cast<int>(points.Size() >> 1),
        verbs.Address(),
        static_cast<int>(verbs.Size()),
        weights.Address(),
        static_cast<int>(weights.Size()),
        *fill_type, is_volatile
    );
    if (path.isEmpty())
        return ffi::Fail(ffi::kErr, "invalid sequence of path commands");

    return ffi::JSObject::New<Path>(v8::Isolate::GetCurrent(), path);
}

ffi::RetLocal<v8::Value> Path::Rect(RectAdapter rect,
                                    const ffi::Enum<SkPathDirection>& dir,
                                    uint32_t start_index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, SkPath::Rect(*rect, *dir, start_index));
}

ffi::RetLocal<v8::Value> Path::Oval(RectAdapter rect,
                                    const ffi::Enum<SkPathDirection>& dir,
                                    uint32_t start_index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, SkPath::Oval(*rect, *dir, start_index));
}

ffi::RetLocal<v8::Value> Path::Circle(float center_x,
                                      float center_y,
                                      float radius,
                                      const ffi::Enum<SkPathDirection>& dir)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, SkPath::Circle(center_x, center_y, radius, *dir));
}

ffi::RetLocal<v8::Value> Path::RRect(RRectAdapter rrect,
                                     const ffi::Enum<SkPathDirection>& dir,
                                     uint32_t start_index)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, SkPath::RRect(*rrect, *dir, start_index));
}

ffi::RetLocal<v8::Value> Path::Polygon(const ffi::Mem<float>& points,
                                       bool is_closed,
                                       const ffi::Enum<SkPathFillType> &fill_type,
                                       bool is_volatile)
{
    if (points.Size() & 1)
        return ffi::Fail(ffi::kErr, "invalid length of array as flattened points");

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, SkPath::Polygon(
            reinterpret_cast<SkPoint*>(points.Address()),
            static_cast<int>(points.Size() >> 1),
            is_closed,
            *fill_type,
            is_volatile));
}

ffi::RetLocal<v8::Value> Path::Line(float x1, float y1, float x2, float y2)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, SkPath::Line({x1, y1}, {x2, y2}));
}

ffi::RetLocal<v8::Value> Path::clone()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, path_);
}

ffi::Ret<bool> Path::equalTo(const ffi::Class<Path>& other)
{
    return (path_ == (*other)->GetSkPath());
}

ffi::Ret<bool> Path::isInterpolatable(const ffi::Class<Path>& compare)
{
    return path_.isInterpolatable((*compare)->path_);
}

ffi::RetLocal<v8::Value> Path::interpolate(const ffi::Class<Path>& ending, float weight)
{
    SkPath receiver;
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!path_.interpolate((*ending)->path_, weight, &receiver))
        return v8::Null(isolate);
    return ffi::JSObject::New<Path>(isolate, receiver);
}

ffi::Ret<void> Path::toggleInverseFillType()
{
    path_.toggleInverseFillType();
    return {};
}

ffi::RetLocal<v8::Value> Path::asOval()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkRect result;
    if (!path_.isOval(&result))
        return v8::Null(isolate);
    return CreateJSRect(isolate, result);
}

ffi::RetLocal<v8::Value> Path::asRRect()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkRRect result;
    if (!path_.isRRect(&result))
        return v8::Null(isolate);
    return CreateJSRRect(isolate, result);
}

ffi::Ret<void> Path::reset()
{
    path_.reset();
    return {};
}

ffi::Ret<void> Path::rewind()
{
    path_.rewind();
    return {};
}

ffi::Ret<int32_t> Path::countPoints()
{
    return path_.countPoints();
}

ffi::Ret<std::tuple<float, float>> Path::getPoint(int32_t index)
{
    SkPoint pt = path_.getPoint(index);
    return std::make_tuple(pt.fX, pt.fY);
}

ffi::Ret<int32_t> Path::getPoints(const ffi::Mem<float>& dst, int32_t max)
{
    if (max * 2 > dst.Size())
        return ffi::Fail(ffi::kErr, "destination buffer too small");
    return path_.getPoints(reinterpret_cast<SkPoint*>(dst.Address()), max);
}

ffi::Ret<int32_t> Path::countVerbs()
{
    return path_.countVerbs();
}


ffi::Ret<int32_t> Path::getVerbs(const ffi::Mem<uint8_t>& dst, int32_t max)
{
    if (max > dst.Size())
        return ffi::Fail(ffi::kErr, "destination buffer too small");
    return path_.getVerbs(dst.Address(), max);
}

ffi::RetLocal<v8::Value> Path::getRoughBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CreateJSRect(isolate, path_.getBounds());
}

ffi::RetLocal<v8::Value> Path::computeTightBounds()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return CreateJSRect(isolate, path_.computeTightBounds());
}

ffi::Ret<bool> Path::conservativelyContainsRect(RectAdapter rect)
{
    return path_.conservativelyContainsRect(*rect);
}

ffi::RetLocal<v8::Value> Path::asRect()
{
    SkRect rect;
    bool closed;
    SkPathDirection dir;

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!path_.isRect(&rect, &closed, &dir))
        return v8::Null(isolate);

    auto tuple = std::make_tuple(CreateJSRect(isolate, rect), closed, static_cast<int>(dir));
    return ffi::Cast<decltype(tuple)>::ToChecked(isolate, tuple);
}

ffi::Ret<void> Path::addPathOffset(const ffi::Class<Path>& src,
                                   float dx, float dy,
                                   const ffi::Enum<SkPath::AddPathMode>& mode)
{
    path_.addPath(src->GetSkPath(), dx, dy, *mode);
    return {};
}

ffi::Ret<void> Path::addPath(const ffi::Class<Path>& src,
                             const Mat3x3Adapter& matrix,
                             const ffi::Enum<SkPath::AddPathMode>& mode)
{
    path_.addPath(src->GetSkPath(), *matrix, *mode);
    return {};
}

ffi::Ret<void> Path::reverseAddPath(const ffi::Class<Path>& src)
{
    path_.reverseAddPath(src->GetSkPath());
    return {};
}

ffi::Ret<void> Path::transform(const Mat3x3Adapter& matrix, bool perspective_clip)
{
    path_.transform(*matrix, perspective_clip ? SkApplyPerspectiveClip::kYes
                                              : SkApplyPerspectiveClip::kNo);
    return {};
}

ffi::RetLocal<v8::Value> Path::makeTransform(const Mat3x3Adapter& matrix, bool perspective_clip)
{
    SkPath path = path_.makeTransform(*matrix, perspective_clip ? SkApplyPerspectiveClip::kYes
                                                                : SkApplyPerspectiveClip::kNo);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::JSObject::New<Path>(isolate, path);
}

ffi::Ret<bool> Path::contains(float x, float y)
{
    return path_.contains(x, y);
}

ffi::RetLocal<v8::Value> Path::fillWithPaint(const ffi::Class<Paint>& paint,
                                             const ffi::Opt<RectAdapter>& cull,
                                             const ffi::Opt<Mat3x3Adapter>& ctm)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    SkPath dst;
    if (!skpathutils::FillPathWithPaint(path_, paint->GetSkPaint(), &dst,
                                        cull ? &(**cull) : nullptr,
                                        ctm ? **ctm : SkMatrix::I()))
    {
        return v8::Null(isolate);
    }
    return ffi::JSObject::New<Path>(isolate, dst);
}

ffi::Ret<ffi::Mem<uint8_t>> Path::serialize()
{
    sk_sp<SkData> data = path_.serialize();
    CHECK(data && data->unique());

    void *ptr = data->writable_data();
    size_t size = data->size();
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return ffi::Mem<uint8_t>(isolate, std::move(data), ptr, size);
}

ffi::Ret<size_t> Path::serializeToMemory(const ffi::Mem<uint8_t>& buffer)
{
    size_t required_size = path_.writeToMemory(nullptr);
    if (buffer.ByteSize() < required_size)
        return ffi::Fail(ffi::kErr, "the provided buffer is too small");
    return path_.writeToMemory(buffer.Address());
}

ffi::Ret<std::tuple<v8::Local<v8::Value>, size_t>>
Path::Deserialize(const ffi::Mem<uint8_t>& buffer)
{
    if ((buffer.ByteSize() & 0b11) != 0)
        return ffi::Fail(ffi::kErr, "size of buffer must be the multiple of 4");
    SkPath receiver;
    size_t read = receiver.readFromMemory(buffer.Address(), buffer.ByteSize());

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (read == 0)
        return std::make_tuple<v8::Local<v8::Value>, size_t>(v8::Null(isolate), 0);

    v8::Local<v8::Value> result = ffi::JSObject::New<Path>(isolate, receiver);
    return std::make_tuple(result, read);
}

ffi::Ret<bool> Path::isValid()
{
    return path_.isValid();
}

GALLIUM_BINDINGS_RENDERER_NS_END
