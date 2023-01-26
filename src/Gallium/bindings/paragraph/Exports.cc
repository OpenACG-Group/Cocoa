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

#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"

#include "Gallium/bindings/paragraph/Exports.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDINGS_PARAGRAPH_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
#define EV(v) static_cast<int32_t>(v)

    namespace P = skia::textlayout;

    std::unordered_map<std::string_view, int32_t> constants{
        { "RECT_HEIGHT_STYLE_TIGHT", EV(P::RectHeightStyle::kTight) },
        { "RECT_HEIGHT_STYLE_MAX", EV(P::RectHeightStyle::kMax) },
        { "RECT_HEIGHT_STYLE_INCLUDE_LINE_SPACING_MIDDLE", EV(P::RectHeightStyle::kIncludeLineSpacingMiddle) },
        { "RECT_HEIGHT_STYLE_INCLUDE_LINE_SPACING_TOP", EV(P::RectHeightStyle::kIncludeLineSpacingTop) },
        { "RECT_HEIGHT_STYLE_INCLUDE_LINE_SPACING_BOTTOM", EV(P::RectHeightStyle::kIncludeLineSpacingBottom) },
        { "RECT_HEIGHT_STYLE_STRUT", EV(P::RectHeightStyle::kStrut) },

        { "RECT_WIDTH_STYLE_TIGHT", EV(P::RectWidthStyle::kTight) },
        { "RECT_WIDTH_STYLE_MAX", EV(P::RectWidthStyle::kMax) },

        { "TEXT_ALIGN_LEFT", EV(P::TextAlign::kLeft) },
        { "TEXT_ALIGN_RIGHT", EV(P::TextAlign::kRight) },
        { "TEXT_ALIGN_CENTER", EV(P::TextAlign::kCenter) },
        { "TEXT_ALIGN_JUSTIFY", EV(P::TextAlign::kJustify) },
        { "TEXT_ALIGN_START", EV(P::TextAlign::kStart) },
        { "TEXT_ALIGN_END", EV(P::TextAlign::kEnd) },

        { "TEXT_DIRECTION_RTL", EV(P::TextDirection::kRtl) },
        { "TEXT_DIRECTION_LTR", EV(P::TextDirection::kLtr) },

        { "TEXT_BASELINE_ALPHABETIC", EV(P::TextBaseline::kAlphabetic) },
        { "TEXT_BASELINE_IDEOGRAPHIC", EV(P::TextBaseline::kIdeographic) },

        { "TEXT_HEIGHT_BEHAVIOR_ALL", EV(P::TextHeightBehavior::kAll) },
        { "TEXT_HEIGHT_BEHAVIOR_DISABLE_FIRST_ASCENT", EV(P::TextHeightBehavior::kDisableFirstAscent) },
        { "TEXT_HEIGHT_BEHAVIOR_DISABLE_LAST_DESCENT", EV(P::TextHeightBehavior::kDisableLastDescent) },
        { "TEXT_HEIGHT_BEHAVIOR_DISABLE_ALL", EV(P::TextHeightBehavior::kDisableAll) },

        { "LINE_METRIC_STYLE_TYPOGRAPHIC", EV(P::LineMetricStyle::Typographic) },
        { "LINE_METRIC_STYLE_CSS", EV(P::LineMetricStyle::CSS) },

        { "TEXT_DECORATION_NO_DECORATION", EV(P::TextDecoration::kNoDecoration) },
        { "TEXT_DECORATION_UNDERLINE", EV(P::TextDecoration::kUnderline) },
        { "TEXT_DECORATION_OVERLINE", EV(P::TextDecoration::kOverline) },
        { "TEXT_DECORATION_LINE_THROUGH", EV(P::TextDecoration::kLineThrough) },

        { "TEXT_DECORATION_STYLE_SOLID", EV(P::TextDecorationStyle::kSolid) },
        { "TEXT_DECORATION_STYLE_DOUBLE", EV(P::TextDecorationStyle::kDouble) },
        { "TEXT_DECORATION_STYLE_DOTTED", EV(P::TextDecorationStyle::kDotted) },
        { "TEXT_DECORATION_STYLE_DASHED", EV(P::TextDecorationStyle::kDashed) },
        { "TEXT_DECORATION_STYLE_WAVY", EV(P::TextDecorationStyle::kWavy) },

        { "TEXT_DECORATION_MODE_GAPS", EV(P::TextDecorationMode::kGaps) },
        { "TEXT_DECORATION_MODE_THROUGH", EV(P::TextDecorationMode::kThrough) },

        { "STYLE_TYPE_NONE", EV(P::StyleType::kNone) },
        { "STYLE_TYPE_ALL_ATTRIBUTES", EV(P::StyleType::kAllAttributes) },
        { "STYLE_TYPE_FONT", EV(P::StyleType::kFont) },
        { "STYLE_TYPE_FOREGROUND", EV(P::StyleType::kForeground) },
        { "STYLE_TYPE_BACKGROUND", EV(P::StyleType::kBackground) },
        { "STYLE_TYPE_SHADOW", EV(P::StyleType::kShadow) },
        { "STYLE_TYPE_DECORATIONS", EV(P::StyleType::kDecorations) },
        { "STYLE_TYPE_LETTER_SPACING", EV(P::StyleType::kLetterSpacing) },
        { "STYLE_TYPE_WORD_SPACING", EV(P::StyleType::kWordSpacing) },

        { "PLACEHOLDER_ALIGNMENT_BASELINE", EV(P::PlaceholderAlignment::kBaseline) },
        { "PLACEHOLDER_ALIGNMENT_ABOVE_BASELINE", EV(P::PlaceholderAlignment::kAboveBaseline) },
        { "PLACEHOLDER_ALIGNMENT_BELOW_BASELINE", EV(P::PlaceholderAlignment::kBelowBaseline) },
        { "PLACEHOLDER_ALIGNMENT_TOP", EV(P::PlaceholderAlignment::kTop) },
        { "PLACEHOLDER_ALIGNMENT_BOTTOM", EV(P::PlaceholderAlignment::kBottom) },
        { "PLACEHOLDER_ALIGNMENT_MIDDLE", EV(P::PlaceholderAlignment::kMiddle) },

        { "AFFINITY_UPSTREAM", EV(P::Affinity::kUpstream) },
        { "AFFINITY_DOWNSTREAM", EV(P::Affinity::kDownstream) }
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    instance->Set(ctx, binder::to_v8(isolate, "Constants"), binder::to_v8(isolate, constants)).Check();
}

GALLIUM_BINDINGS_PARAGRAPH_NS_END
