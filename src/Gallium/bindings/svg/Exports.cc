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

#include "include/svg/SkSVGCanvas.h"
#include "modules/svg/include/SkSVGTypes.h"
#include "modules/svg/include/SkSVGSVG.h"

#include "Gallium/bindings/svg/Exports.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDINGS_SVG_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance)
{
#define V(x) static_cast<uint32_t>(x)
    std::unordered_map<std::string_view, uint32_t> constants = {
        { "SVG_CANVAS_FLAG_CONVERT_TEXT_TO_PATHS", V(SkSVGCanvas::kConvertTextToPaths_Flag) },
        { "SVG_CANVAS_FLAG_NO_PRETTY_XML", V(SkSVGCanvas::kNoPrettyXML_Flag) },
        { "SVG_CANVAS_FLAG_RELATIVE_PATH_ENCODING", V(SkSVGCanvas::kRelativePathEncoding_Flag) },

        { "SVG_LENGTH_UNIT_UNKNOWN", V(SkSVGLength::Unit::kUnknown) },
        { "SVG_LENGTH_UNIT_NUMBER", V(SkSVGLength::Unit::kNumber) },
        { "SVG_LENGTH_UNIT_PERCENTAGE", V(SkSVGLength::Unit::kPercentage) },
        { "SVG_LENGTH_UNIT_EMS", V(SkSVGLength::Unit::kEMS) },
        { "SVG_LENGTH_UNIT_EXS", V(SkSVGLength::Unit::kEXS) },
        { "SVG_LENGTH_UNIT_PX", V(SkSVGLength::Unit::kPX) },
        { "SVG_LENGTH_UNIT_CM", V(SkSVGLength::Unit::kCM) },
        { "SVG_LENGTH_UNIT_MM", V(SkSVGLength::Unit::kMM) },
        { "SVG_LENGTH_UNIT_IN", V(SkSVGLength::Unit::kIN) },
        { "SVG_LENGTH_UNIT_PT", V(SkSVGLength::Unit::kPT) },
        { "SVG_LENGTH_UNIT_PC", V(SkSVGLength::Unit::kPC) },

        { "SVG_LENGTH_TYPE_VERTICAL", V(SkSVGLengthContext::LengthType::kVertical) },
        { "SVG_LENGTH_TYPE_HORIZONTAL", V(SkSVGLengthContext::LengthType::kHorizontal) },
        { "SVG_LENGTH_TYPE_OTHER", V(SkSVGLengthContext::LengthType::kOther) },

        { "SVG_LENGTH_DEFAULT_DPI", 90 },

        { "SVG_TAG_CIRCLE", V(SkSVGTag::kCircle) },
        { "SVG_TAG_CLIP_PATH", V(SkSVGTag::kClipPath) },
        { "SVG_TAG_DEFS", V(SkSVGTag::kDefs) },
        { "SVG_TAG_ELLIPSE", V(SkSVGTag::kEllipse) },
        { "SVG_TAG_FE_BLEND", V(SkSVGTag::kFeBlend) },
        { "SVG_TAG_FE_COLOR_MATRIX", V(SkSVGTag::kFeColorMatrix) },
        { "SVG_TAG_FE_COMPOSITE", V(SkSVGTag::kFeComposite) },
        { "SVG_TAG_FE_DIFFUSE_LIGHTING", V(SkSVGTag::kFeDiffuseLighting) },
        { "SVG_TAG_FE_DISPLACEMENT_MAP", V(SkSVGTag::kFeDisplacementMap) },
        { "SVG_TAG_FE_DISTANT_LIGHT", V(SkSVGTag::kFeDistantLight) },
        { "SVG_TAG_FE_FLOOD", V(SkSVGTag::kFeFlood) },
        { "SVG_TAG_FE_GAUSSIAN_BLUR", V(SkSVGTag::kFeGaussianBlur) },
        { "SVG_TAG_FE_IMAGE", V(SkSVGTag::kFeImage) },
        { "SVG_TAG_FE_MORPHOLOGY", V(SkSVGTag::kFeMorphology) },
        { "SVG_TAG_FE_OFFSET", V(SkSVGTag::kFeOffset) },
        { "SVG_TAG_FE_POINT_LIGHT", V(SkSVGTag::kFePointLight) },
        { "SVG_TAG_FE_SPECULAR_LIGHTING", V(SkSVGTag::kFeSpecularLighting) },
        { "SVG_TAG_FE_SPOT_LIGHT", V(SkSVGTag::kFeSpotLight) },
        { "SVG_TAG_FE_TURBULENCE", V(SkSVGTag::kFeTurbulence) },
        { "SVG_TAG_FILTER", V(SkSVGTag::kFilter) },
        { "SVG_TAG_G", V(SkSVGTag::kG) },
        { "SVG_TAG_IMAGE", V(SkSVGTag::kImage) },
        { "SVG_TAG_LINE", V(SkSVGTag::kLine) },
        { "SVG_TAG_LINEAR_GRADIENT", V(SkSVGTag::kLinearGradient) },
        { "SVG_TAG_MASK", V(SkSVGTag::kMask) },
        { "SVG_TAG_PATH", V(SkSVGTag::kPath) },
        { "SVG_TAG_PATTERN", V(SkSVGTag::kPattern) },
        { "SVG_TAG_POLYGON", V(SkSVGTag::kPolygon) },
        { "SVG_TAG_POLYLINE", V(SkSVGTag::kPolyline) },
        { "SVG_TAG_RADIAL_GRADIENT", V(SkSVGTag::kRadialGradient) },
        { "SVG_TAG_RECT", V(SkSVGTag::kRect) },
        { "SVG_TAG_STOP", V(SkSVGTag::kStop) },
        { "SVG_TAG_SVG", V(SkSVGTag::kSvg) },
        { "SVG_TAG_TEXT", V(SkSVGTag::kText) },
        { "SVG_TAG_TEXT_LITERAL", V(SkSVGTag::kTextLiteral) },
        { "SVG_TAG_TEXT_PATH", V(SkSVGTag::kPath) },
        { "SVG_TAG_T_SPAN", V(SkSVGTag::kTSpan) },
        { "SVG_TAG_USE", V(SkSVGTag::kUse) },

        { "SVG_NODE_TYPE_INNER", V(SkSVGSVG::Type::kInner) },
        { "SVG_NODE_TYPE_ROOT", V(SkSVGSVG::Type::kRoot) }
    };

    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();
    v8::Local<v8::Object> constants_object = binder::to_v8(isolate, constants);

    instance->Set(ctx, v8::String::NewFromUtf8Literal(isolate, "Constants"),
                  constants_object).Check();
}

GALLIUM_BINDINGS_SVG_NS_END
