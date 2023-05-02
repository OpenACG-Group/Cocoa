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

#include <unordered_map>

#include "modules/skparagraph/include/Paragraph.h"

#include "Gallium/bindings/paragraph/Exports.h"
#include "Gallium/bindings/glamor/CkCanvasWrap.h"
#include "Gallium/bindings/glamor/TrivialInterface.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

namespace para = skia::textlayout;

void ParagraphWrap::layout(SkScalar width)
{
    paragraph_->layout(width);
}

void ParagraphWrap::paint(v8::Local<v8::Value> canvas, SkScalar x, SkScalar y)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto *w = binder::Class<glamor_wrap::CkCanvas>::unwrap_object(isolate, canvas);
    if (!w)
        g_throw(TypeError, "Argument `canvas` must be a `glamor.CkCanvas` object");

    paragraph_->paint(w->GetCanvas(), x, y);
}

namespace {

v8::Local<v8::Value> wrap_text_box_array(const std::vector<para::TextBox>& boxes)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();

    if (boxes.empty())
        return v8::Array::New(isolate);

    size_t length = boxes.size();
    v8::Local<v8::Array> array = v8::Array::New(isolate, static_cast<int32_t>(length));
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    for (size_t i = 0; i < length; i++)
    {
        std::unordered_map<std::string_view, v8::Local<v8::Value>> map{
                { "rect", glamor_wrap::NewCkRect(isolate, boxes[i].rect) },
                { "direction", binder::to_v8(isolate, static_cast<int32_t>(boxes[i].direction)) }
        };
        array->Set(ctx, static_cast<int32_t>(i), binder::to_v8(isolate, map)).Check();
    }

    return array;
}

} // namespace anonymous

v8::Local<v8::Value> ParagraphWrap::getRectsForRange(int32_t start, int32_t end,
                                                     int32_t hstyle, int32_t wstyle)
{
    if (start < 0 || end < 0)
        g_throw(RangeError, "Invalid [start, end) argument");
    if (hstyle < 0 || hstyle > static_cast<int32_t>(para::RectHeightStyle::kStrut))
        g_throw(RangeError, "Invalid enumeration value for argument `hStyle`");
    if (wstyle < 0 || wstyle > static_cast<int32_t>(para::RectWidthStyle::kMax))
        g_throw(RangeError, "Invalid enumeration value for argument `wStyle`");

    std::vector<para::TextBox> boxes = paragraph_->getRectsForRange(
            start, end, static_cast<para::RectHeightStyle>(hstyle), static_cast<para::RectWidthStyle>(wstyle));

    return wrap_text_box_array(boxes);
}

v8::Local<v8::Value> ParagraphWrap::getRectsForPlaceholders()
{
    std::vector<para::TextBox> boxes = paragraph_->getRectsForPlaceholders();
    return wrap_text_box_array(boxes);
}

v8::Local<v8::Value> ParagraphWrap::getWordBoundary(int32_t offset)
{
    if (offset < 0)
        g_throw(RangeError, "Invalid value for argument `offset`");

    para::SkRange<size_t> range = paragraph_->getWordBoundary(offset);
    return binder::to_v8(v8::Isolate::GetCurrent(),
                         std::vector<size_t>{range.start, range.end});
}

v8::Local<v8::Value> ParagraphWrap::getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy)
{
    para::PositionWithAffinity pos = paragraph_->getGlyphPositionAtCoordinate(dx, dy);
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    return binder::to_v8(isolate, std::unordered_map<std::string_view, v8::Local<v8::Value>>{
        { "position", binder::to_v8(isolate, pos.position) },
        { "affinity", binder::to_v8(isolate, static_cast<int32_t>(pos.affinity)) }
    });
}

GALLIUM_BINDINGS_PARAGRAPH_NS_END
