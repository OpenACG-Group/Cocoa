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

#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFont.h"
#include "include/core/SkPathMeasure.h"
#include "include/effects/Sk1DPathEffect.h"
#include "include/effects/SkTrimPathEffect.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/core/SkVertices.h"

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/Scene.h"
#include "Gallium/bindings/glamor/CkFontMgrWrap.h"
GALLIUM_BINDINGS_GLAMOR_NS_BEGIN

#define EV(x) static_cast<uint32_t>(x)

void GlamorSetInstanceProperties(v8::Local<v8::Object> instance)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> ctx = isolate->GetCurrentContext();

    using T = SkColorType;
    using A = SkAlphaType;
    using KEY = gl::KeyboardKey;

    std::map<std::string, uint32_t> constants{
        { "CAPABILITY_HWCOMPOSE_ENABLED",       EV(Capabilities::kHWComposeEnabled)   },
        { "CAPABILITY_PROFILER_ENABLED",        EV(Capabilities::kProfilerEnabled)    },
        { "CAPABILITY_PROFILER_MAX_SAMPLES",    EV(Capabilities::kProfilerMaxSamples) },
        { "CAPABILITY_MESSAGE_QUEUE_PROFILING_ENABLED",
                                                EV(Capabilities::kMessageQueueProfilingEnabled) },

        { "COLOR_TYPE_ALPHA8",              EV(T::kAlpha_8_SkColorType)             },
        { "COLOR_TYPE_RGB565",              EV(T::kRGB_565_SkColorType)             },
        { "COLOR_TYPE_ARGB4444",            EV(T::kARGB_4444_SkColorType)           },
        { "COLOR_TYPE_RGBA8888",            EV(T::kRGBA_8888_SkColorType)           },
        { "COLOR_TYPE_RGB888x",             EV(T::kRGB_888x_SkColorType)            },
        { "COLOR_TYPE_BGRA8888",            EV(T::kBGRA_8888_SkColorType)           },
        { "COLOR_TYPE_BGRA1010102",         EV(T::kBGRA_1010102_SkColorType)        },
        { "COLOR_TYPE_RGBA1010102",         EV(T::kRGBA_1010102_SkColorType)        },
        { "COLOR_TYPE_RGB101010x",          EV(T::kRGB_101010x_SkColorType)         },
        { "COLOR_TYPE_BGR101010x",          EV(T::kBGR_101010x_SkColorType)         },
        { "COLOR_TYPE_GRAY8",               EV(T::kGray_8_SkColorType)              },
        { "COLOR_TYPE_RGBA_F16_NORM",       EV(T::kRGBA_F16Norm_SkColorType)        },
        { "COLOR_TYPE_RGBA_F16",            EV(T::kRGBA_F16_SkColorType)            },
        { "COLOR_TYPE_RGBA_F32",            EV(T::kRGBA_F32_SkColorType)            },
        { "COLOR_TYPE_R8G8_UNORM",          EV(T::kR8G8_unorm_SkColorType)          },
        { "COLOR_TYPE_A16_FLOAT",           EV(T::kA16_float_SkColorType)           },
        { "COLOR_TYPE_R16G16_FLOAT",        EV(T::kR16G16_float_SkColorType)        },
        { "COLOR_TYPE_A16_UNORM",           EV(T::kA16_unorm_SkColorType)           },
        { "COLOR_TYPE_R16G16_UNORM",        EV(T::kR16G16_unorm_SkColorType)        },
        { "COLOR_TYPE_R16G16B16A16_UNORM",  EV(T::kR16G16B16A16_unorm_SkColorType)  },

        { "ALPHA_TYPE_PREMULTIPLIED",   EV(A::kPremul_SkAlphaType)   },
        { "ALPHA_TYPE_UNPREMULTIPLIED", EV(A::kUnpremul_SkAlphaType) },
        { "ALPHA_TYPE_OPAQUE",          EV(A::kOpaque_SkAlphaType)   },

        { "COLOR_SPACE_SRGB", EV(ColorSpace::kSRGB) },

        { "PAINT_STYLE_FILL", EV(SkPaint::kFill_Style) },
        { "PAINT_STYLE_STROKE", EV(SkPaint::kStroke_Style) },
        { "PAINT_STYLE_STROKE_FILL", EV(SkPaint::kStrokeAndFill_Style) },
        { "PAINT_CAP_BUTT", EV(SkPaint::kButt_Cap) },
        { "PAINT_CAP_ROUND", EV(SkPaint::kRound_Cap) },
        { "PAINT_CAP_SQUARE", EV(SkPaint::kSquare_Cap) },
        { "PAINT_JOIN_MITER", EV(SkPaint::kMiter_Join) },
        { "PAINT_JOIN_ROUND", EV(SkPaint::kRound_Join) },
        { "PAINT_JOIN_BEVEL", EV(SkPaint::kBevel_Join) },

        { "PATH_FILL_TYPE_WINDING", EV(SkPathFillType::kWinding) },
        { "PATH_FILL_TYPE_EVEN_ODD", EV(SkPathFillType::kEvenOdd) },
        { "PATH_FILL_TYPE_INVERSE_WINDING", EV(SkPathFillType::kInverseWinding) },
        { "PATH_FILL_TYPE_INVERSE_EVEN_ODD", EV(SkPathFillType::kInverseEvenOdd) },
        { "PATH_DIRECTION_CW", EV(SkPathDirection::kCW) },
        { "PATH_DIRECTION_CCW", EV(SkPathDirection::kCCW) },
        { "PATH_ARC_SIZE_SMALL", EV(SkPath::ArcSize::kSmall_ArcSize) },
        { "PATH_ARC_SIZE_LARGE", EV(SkPath::ArcSize::kLarge_ArcSize) },
        { "PATH_ADD_PATH_MODE_APPEND", EV(SkPath::AddPathMode::kAppend_AddPathMode) },
        { "PATH_ADD_PATH_MODE_EXTEND", EV(SkPath::AddPathMode::kExtend_AddPathMode) },

        { "PATH_MEASURE_MATRIX_FLAGS_GET_POSITION", EV(SkPathMeasure::kGetPosition_MatrixFlag) },
        { "PATH_MEASURE_MATRIX_FLAGS_GET_TANGENT",  EV(SkPathMeasure::kGetTangent_MatrixFlag) },

        { "APPLY_PERSPECTIVE_CLIP_YES", EV(SkApplyPerspectiveClip::kYes) },
        { "APPLY_PERSPECTIVE_CLIP_NO", EV(SkApplyPerspectiveClip::kNo) },

        { "MATRIX_SCALE_TO_FIT_FILL", EV(SkMatrix::kFill_ScaleToFit) },
        { "MATRIX_SCALE_TO_FIT_START", EV(SkMatrix::kStart_ScaleToFit) },
        { "MATRIX_SCALE_TO_FIT_CENTER", EV(SkMatrix::kCenter_ScaleToFit) },
        { "MATRIX_SCALE_TO_FIT_END", EV(SkMatrix::kEnd_ScaleToFit) },

        { "CANVAS_SAVE_LAYER_PRESERVE_LCD_TEXT", EV(SkCanvas::kPreserveLCDText_SaveLayerFlag) },
        { "CANVAS_SAVE_LAYER_INIT_WITH_PREVIOUS", EV(SkCanvas::kInitWithPrevious_SaveLayerFlag) },
        { "CANVAS_SAVE_LAYER_F16_COLOR_TYPE", EV(SkCanvas::kF16ColorType) },
        { "CANVAS_POINT_MODE_POINTS", EV(SkCanvas::kPoints_PointMode) },
        { "CANVAS_POINT_MODE_LINES", EV(SkCanvas::kLines_PointMode) },
        { "CANVAS_POINT_MODE_POLYGON", EV(SkCanvas::kPolygon_PointMode) },
        { "CANVAS_SRC_RECT_CONSTRAINT_STRICT", EV(SkCanvas::kStrict_SrcRectConstraint) },
        { "CANVAS_SRC_RECT_CONSTRAINT_FAST", EV(SkCanvas::kFast_SrcRectConstraint) },

        { "CLIP_OP_DIFFERENCE", EV(SkClipOp::kDifference) },
        { "CLIP_OP_INTERSECT", EV(SkClipOp::kIntersect) },

        { "FONT_STYLE_WEIGHT_INVISIBLE", EV(SkFontStyle::kInvisible_Weight) },
        { "FONT_STYLE_WEIGHT_THIN", EV(SkFontStyle::kThin_Weight) },
        { "FONT_STYLE_WEIGHT_EXTRA_LIGHT", EV(SkFontStyle::kExtraLight_Weight) },
        { "FONT_STYLE_WEIGHT_LIGHT", EV(SkFontStyle::kLight_Weight) },
        { "FONT_STYLE_WEIGHT_NORMAL", EV(SkFontStyle::kNormal_Weight) },
        { "FONT_STYLE_WEIGHT_MEDIUM", EV(SkFontStyle::kMedium_Weight) },
        { "FONT_STYLE_WEIGHT_SEMI_BOLD", EV(SkFontStyle::kSemiBold_Weight) },
        { "FONT_STYLE_WEIGHT_BOLD", EV(SkFontStyle::kBold_Weight) },
        { "FONT_STYLE_WEIGHT_EXTRA_BOLD", EV(SkFontStyle::kExtraBold_Weight) },
        { "FONT_STYLE_WEIGHT_BLACK", EV(SkFontStyle::kBlack_Weight) },
        { "FONT_STYLE_WEIGHT_EXTRA_BLACK", EV(SkFontStyle::kExtraBlack_Weight) },
        { "FONT_STYLE_WIDTH_ULTRA_CONDENSED", EV(SkFontStyle::kUltraCondensed_Width) },
        { "FONT_STYLE_WIDTH_EXTRA_CONDENSED", EV(SkFontStyle::kExtraCondensed_Width) },
        { "FONT_STYLE_WIDTH_CONDENSED", EV(SkFontStyle::kCondensed_Width) },
        { "FONT_STYLE_WIDTH_SEMI_CONDENSED", EV(SkFontStyle::kSemiCondensed_Width) },
        { "FONT_STYLE_WIDTH_NORMAL", EV(SkFontStyle::kNormal_Width) },
        { "FONT_STYLE_WIDTH_SEMI_EXPANDED", EV(SkFontStyle::kSemiExpanded_Width) },
        { "FONT_STYLE_WIDTH_EXPANDED", EV(SkFontStyle::kExpanded_Width) },
        { "FONT_STYLE_WIDTH_EXTRA_EXPANDED", EV(SkFontStyle::kExtraExpanded_Width) },
        { "FONT_STYLE_WIDTH_ULTRA_EXPANDED", EV(SkFontStyle::kUltraExpanded_Width) },
        { "FONT_STYLE_SLANT_UPRIGHT", EV(SkFontStyle::kUpright_Slant) },
        { "FONT_STYLE_SLANT_ITALIC", EV(SkFontStyle::kItalic_Slant) },
        { "FONT_STYLE_SLANT_OBLIQUE", EV(SkFontStyle::kOblique_Slant) },

        { "FONT_EDGING_ALIAS", EV(SkFont::Edging::kAlias) },
        { "FONT_EDGING_ANTIALIAS", EV(SkFont::Edging::kAntiAlias) },
        { "FONT_EDGING_SUBPIXEL_ANTIALIAS", EV(SkFont::Edging::kSubpixelAntiAlias) },

        { "FONT_HINTING_NONE", EV(SkFontHinting::kNone) },
        { "FONT_HINTING_SLIGHT", EV(SkFontHinting::kSlight) },
        { "FONT_HINTING_NORMAL", EV(SkFontHinting::kNormal) },
        { "FONT_HINTING_FULL", EV(SkFontHinting::kFull) },

        { "TEXT_ENCODING_UTF8", EV(SkTextEncoding::kUTF8) },
        { "TEXT_ENCODING_UTF16", EV(SkTextEncoding::kUTF16) },
        { "TEXT_ENCODING_UTF32", EV(SkTextEncoding::kUTF32) },

        { "PATH_EFFECT_PATH1D_STYLE_TRANSLATE", EV(SkPath1DPathEffect::kTranslate_Style) },
        { "PATH_EFFECT_PATH1D_STYLE_ROTATE", EV(SkPath1DPathEffect::kRotate_Style) },
        { "PATH_EFFECT_PATH1D_STYLE_MORPH", EV(SkPath1DPathEffect::kMorph_Style) },
        { "PATH_EFFECT_TRIM_NORMAL", EV(SkTrimPathEffect::Mode::kNormal) },
        { "PATH_EFFECT_TRIM_INVERTED", EV(SkTrimPathEffect::Mode::kInverted) },

        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT", EV(SkRuntimeEffect::Uniform::Type::kFloat) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT2", EV(SkRuntimeEffect::Uniform::Type::kFloat2) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT3", EV(SkRuntimeEffect::Uniform::Type::kFloat3) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT4", EV(SkRuntimeEffect::Uniform::Type::kFloat4) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT2X2", EV(SkRuntimeEffect::Uniform::Type::kFloat2x2) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT3X3", EV(SkRuntimeEffect::Uniform::Type::kFloat3x3) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_FLOAT4X4", EV(SkRuntimeEffect::Uniform::Type::kFloat4x4) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_Int", EV(SkRuntimeEffect::Uniform::Type::kInt) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_Int2", EV(SkRuntimeEffect::Uniform::Type::kInt2) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_Int3", EV(SkRuntimeEffect::Uniform::Type::kInt3) },
        { "RUNTIME_EFFECT_UNIFORM_TYPE_Int4", EV(SkRuntimeEffect::Uniform::Type::kInt4) },

        { "RUNTIME_EFFECT_UNIFORM_FLAG_ARRAY", EV(SkRuntimeEffect::Uniform::kArray_Flag) },
        { "RUNTIME_EFFECT_UNIFORM_FLAG_COLOR", EV(SkRuntimeEffect::Uniform::kColor_Flag) },
        { "RUNTIME_EFFECT_UNIFORM_FLAG_VERTEX", EV(SkRuntimeEffect::Uniform::kVertex_Flag) },
        { "RUNTIME_EFFECT_UNIFORM_FLAG_FRAGMENT", EV(SkRuntimeEffect::Uniform::kFragment_Flag) },
        { "RUNTIME_EFFECT_UNIFORM_FLAG_HALF_PRECISION", EV(SkRuntimeEffect::Uniform::kHalfPrecision_Flag) },

        { "RUNTIME_EFFECT_CHILD_TYPE_SHADER", EV(SkRuntimeEffect::ChildType::kShader) },
        { "RUNTIME_EFFECT_CHILD_TYPE_COLOR_FILTER", EV(SkRuntimeEffect::ChildType::kColorFilter) },
        { "RUNTIME_EFFECT_CHILD_TYPE_BLENDER", EV(SkRuntimeEffect::ChildType::kBlender) },

        { "VERTICES_VERTEX_MODE_TRIANGLES", EV(SkVertices::kTriangles_VertexMode) },
        { "VERTICES_VERTEX_MODE_TRIANGLE_STRIP", EV(SkVertices::kTriangleStrip_VertexMode) },
        { "VERTICES_VERTEX_MODE_TRIANGLE_FAN", EV(SkVertices::kTriangleFan_VertexMode) },

        { "FORMAT_PNG",     EV(SkEncodedImageFormat::kPNG)  },
        { "FORMAT_JPEG",    EV(SkEncodedImageFormat::kJPEG) },
        { "FORMAT_WEBP",    EV(SkEncodedImageFormat::kWEBP) },
        { "FORMAT_GIF",     EV(SkEncodedImageFormat::kGIF)  },

        { "SAMPLING_FILTER_NEAREST",    EV(Sampling::kNearest)          },
        { "SAMPLING_FILTER_LINEAR",     EV(Sampling::kLinear)           },
        { "SAMPLING_CUBIC_MITCHELL",    EV(Sampling::kCubicMitchell)    },
        { "SAMPLING_CUBIC_CATMULL_ROM", EV(Sampling::kCubicCatmullRom)  },

        { "TILE_MODE_CLAMP",    EV(SkTileMode::kClamp)  },
        { "TILE_MODE_REPEAT",   EV(SkTileMode::kRepeat) },
        { "TILE_MODE_MIRROR",   EV(SkTileMode::kMirror) },
        { "TILE_MODE_DECAL",    EV(SkTileMode::kDecal)  },

        { "BLEND_MODE_CLEAR",       EV(SkBlendMode::kClear)      },
        { "BLEND_MODE_SRC",         EV(SkBlendMode::kSrc)        },
        { "BLEND_MODE_DST",         EV(SkBlendMode::kDst)        },
        { "BLEND_MODE_SRC_OVER",    EV(SkBlendMode::kSrcOver)    },
        { "BLEND_MODE_DST_OVER",    EV(SkBlendMode::kDstOver)    },
        { "BLEND_MODE_SRC_IN",      EV(SkBlendMode::kSrcIn)      },
        { "BLEND_MODE_DST_IN",      EV(SkBlendMode::kDstIn)      },
        { "BLEND_MODE_SRC_OUT",     EV(SkBlendMode::kSrcOut)     },
        { "BLEND_MODE_DST_OUT",     EV(SkBlendMode::kDstOut)     },
        { "BLEND_MODE_SRC_ATOP",    EV(SkBlendMode::kSrcATop)    },
        { "BLEND_MODE_DST_ATOP",    EV(SkBlendMode::kDstATop)    },
        { "BLEND_MODE_XOR",         EV(SkBlendMode::kXor)        },
        { "BLEND_MODE_PLUS",        EV(SkBlendMode::kPlus)       },
        { "BLEND_MODE_MODULATE",    EV(SkBlendMode::kModulate)   },
        { "BLEND_MODE_SCREEN",      EV(SkBlendMode::kScreen)     },
        { "BLEND_MODE_OVERLAY",     EV(SkBlendMode::kOverlay)    },
        { "BLEND_MODE_DARKEN",      EV(SkBlendMode::kDarken)     },
        { "BLEND_MODE_LIGHTEN",     EV(SkBlendMode::kLighten)    },
        { "BLEND_MODE_COLOR_DODGE", EV(SkBlendMode::kColorDodge) },
        { "BLEND_MODE_COLOR_BURN",  EV(SkBlendMode::kColorBurn)  },
        { "BLEND_MODE_HARD_LIGHT",  EV(SkBlendMode::kHardLight)  },
        { "BLEND_MODE_SOFT_LIGHT",  EV(SkBlendMode::kSoftLight)  },
        { "BLEND_MODE_DIFFERENCE",  EV(SkBlendMode::kDifference) },
        { "BLEND_MODE_EXCLUSION",   EV(SkBlendMode::kExclusion)  },
        { "BLEND_MODE_HUE",         EV(SkBlendMode::kHue)        },
        { "BLEND_MODE_SATURATION",  EV(SkBlendMode::kSaturation) },
        { "BLEND_MODE_COLOR",       EV(SkBlendMode::kColor)      },
        { "BLEND_MODE_LUMINOSITY",  EV(SkBlendMode::kLuminosity) },

        { "POINTER_BUTTON_LEFT",    EV(gl::PointerButton::kLeft)        },
        { "POINTER_BUTTON_RIGHT",   EV(gl::PointerButton::kRight)       },
        { "POINTER_BUTTON_MIDDLE",  EV(gl::PointerButton::kMiddle)      },
        { "POINTER_BUTTON_SIDE",    EV(gl::PointerButton::kSide)        },
        { "POINTER_BUTTON_EXTRA",   EV(gl::PointerButton::kExtra)       },
        { "POINTER_BUTTON_FORWARD", EV(gl::PointerButton::kForward)     },
        { "POINTER_BUTTON_BACK",    EV(gl::PointerButton::kBack)        },
        { "POINTER_BUTTON_TASK",    EV(gl::PointerButton::kTask)        },

        { "POINTER_AXIS_SOURCE_WHEEL", EV(gl::AxisSourceType::kWheel)           },
        { "POINTER_AXIS_SOURCE_WHEEL_TILT", EV(gl::AxisSourceType::kWheelTilt)  },
        { "POINTER_AXIS_SOURCE_FINGER", EV(gl::AxisSourceType::kFinger)         },
        { "POINTER_AXIS_CONTINUOUS", EV(gl::AxisSourceType::kContinuous)        },
        { "POINTER_AXIS_UNKNOWN", EV(gl::AxisSourceType::kUnknown)              },

        { "MODIFIER_CONTROL",       EV(gl::KeyboardModifiers::kControl) },
        { "MODIFIER_ALT",           EV(gl::KeyboardModifiers::kAlt)     },
        { "MODIFIER_SHIFT",         EV(gl::KeyboardModifiers::kShift)   },
        { "MODIFIER_SUPER",         EV(gl::KeyboardModifiers::kSuper)   },
        { "MODIFIER_CAPS_LOCK",     EV(gl::KeyboardModifiers::kCapsLock)},
        { "MODIFIER_NUM_LOCK",      EV(gl::KeyboardModifiers::kNumLock) },
        /* gl::KeyboardModifiers::kMeta is reserved for future */

        { "KEY_SPACE", 				EV(KEY::kKey_SPACE)         },
        { "KEY_APOSTROPHE", 		EV(KEY::kKey_APOSTROPHE)    },
        { "KEY_COMMA", 				EV(KEY::kKey_COMMA)         },
        { "KEY_MINUS", 				EV(KEY::kKey_MINUS)         },
        { "KEY_PERIOD", 			EV(KEY::kKey_PERIOD)        },
        { "KEY_SLASH", 				EV(KEY::kKey_SLASH)         },
        { "KEY_0", 				    EV(KEY::kKey_0) },
        { "KEY_1", 				    EV(KEY::kKey_1) },
        { "KEY_2", 				    EV(KEY::kKey_2) },
        { "KEY_3", 				    EV(KEY::kKey_3) },
        { "KEY_4", 				    EV(KEY::kKey_4) },
        { "KEY_5", 				    EV(KEY::kKey_5) },
        { "KEY_6", 				    EV(KEY::kKey_6) },
        { "KEY_7", 				    EV(KEY::kKey_7) },
        { "KEY_8", 				    EV(KEY::kKey_8) },
        { "KEY_9", 				    EV(KEY::kKey_9) },
        { "KEY_SEMICOLON", 			EV(KEY::kKey_SEMICOLON) },
        { "KEY_EQUAL", 				EV(KEY::kKey_EQUAL)     },
        { "KEY_A", 				    EV(KEY::kKey_A) },
        { "KEY_B", 				    EV(KEY::kKey_B) },
        { "KEY_C", 				    EV(KEY::kKey_C) },
        { "KEY_D", 				    EV(KEY::kKey_D) },
        { "KEY_E", 				    EV(KEY::kKey_E) },
        { "KEY_F", 				    EV(KEY::kKey_F) },
        { "KEY_G", 				    EV(KEY::kKey_G) },
        { "KEY_H", 				    EV(KEY::kKey_H) },
        { "KEY_I", 				    EV(KEY::kKey_I) },
        { "KEY_J", 				    EV(KEY::kKey_J) },
        { "KEY_K", 				    EV(KEY::kKey_K) },
        { "KEY_L", 				    EV(KEY::kKey_L) },
        { "KEY_M", 				    EV(KEY::kKey_M) },
        { "KEY_N", 				    EV(KEY::kKey_N) },
        { "KEY_O", 				    EV(KEY::kKey_O) },
        { "KEY_P", 				    EV(KEY::kKey_P) },
        { "KEY_Q", 				    EV(KEY::kKey_Q) },
        { "KEY_R", 				    EV(KEY::kKey_R) },
        { "KEY_S", 				    EV(KEY::kKey_S) },
        { "KEY_T", 				    EV(KEY::kKey_T) },
        { "KEY_U", 				    EV(KEY::kKey_U) },
        { "KEY_V", 				    EV(KEY::kKey_V) },
        { "KEY_W", 				    EV(KEY::kKey_W) },
        { "KEY_X", 				    EV(KEY::kKey_X) },
        { "KEY_Y", 				    EV(KEY::kKey_Y) },
        { "KEY_Z", 				    EV(KEY::kKey_Z) },
        { "KEY_LEFT_BRACKET", 		EV(KEY::kKey_LEFT_BRACKET)  },
        { "KEY_BACKSLASH", 			EV(KEY::kKey_BACKSLASH)     },
        { "KEY_RIGHT_BRACKET", 		EV(KEY::kKey_RIGHT_BRACKET) },
        { "KEY_GRAVE_ACCENT", 		EV(KEY::kKey_GRAVE_ACCENT)  },
        { "KEY_WORLD_1", 			EV(KEY::kKey_WORLD_1)       },
        { "KEY_WORLD_2", 			EV(KEY::kKey_WORLD_2)       },
        { "KEY_ESCAPE", 			EV(KEY::kKey_ESCAPE)        },
        { "KEY_ENTER", 				EV(KEY::kKey_ENTER)         },
        { "KEY_TAB", 				EV(KEY::kKey_TAB)           },
        { "KEY_BACKSPACE", 			EV(KEY::kKey_BACKSPACE)     },
        { "KEY_INSERT", 			EV(KEY::kKey_INSERT)        },
        { "KEY_DELETE", 			EV(KEY::kKey_DELETE)        },
        { "KEY_RIGHT", 				EV(KEY::kKey_RIGHT)         },
        { "KEY_LEFT", 				EV(KEY::kKey_LEFT)          },
        { "KEY_DOWN", 				EV(KEY::kKey_DOWN)          },
        { "KEY_UP", 				EV(KEY::kKey_UP)            },
        { "KEY_PAGE_UP", 			EV(KEY::kKey_PAGE_UP)       },
        { "KEY_PAGE_DOWN", 			EV(KEY::kKey_PAGE_DOWN)     },
        { "KEY_HOME", 				EV(KEY::kKey_HOME)          },
        { "KEY_END", 				EV(KEY::kKey_END)           },
        { "KEY_CAPS_LOCK", 			EV(KEY::kKey_CAPS_LOCK)     },
        { "KEY_SCROLL_LOCK", 		EV(KEY::kKey_SCROLL_LOCK)   },
        { "KEY_NUM_LOCK", 			EV(KEY::kKey_NUM_LOCK)      },
        { "KEY_PRINT_SCREEN", 		EV(KEY::kKey_PRINT_SCREEN)  },
        { "KEY_PAUSE", 				EV(KEY::kKey_PAUSE)         },
        { "KEY_F1", 				EV(KEY::kKey_F1)    },
        { "KEY_F2", 				EV(KEY::kKey_F2)    },
        { "KEY_F3", 				EV(KEY::kKey_F3)    },
        { "KEY_F4", 				EV(KEY::kKey_F4)    },
        { "KEY_F5", 				EV(KEY::kKey_F5)    },
        { "KEY_F6", 				EV(KEY::kKey_F6)    },
        { "KEY_F7", 				EV(KEY::kKey_F7)    },
        { "KEY_F8", 				EV(KEY::kKey_F8)    },
        { "KEY_F9", 				EV(KEY::kKey_F9)    },
        { "KEY_F10", 				EV(KEY::kKey_F10)   },
        { "KEY_F11", 				EV(KEY::kKey_F11)   },
        { "KEY_F12", 				EV(KEY::kKey_F12)   },
        { "KEY_F13", 				EV(KEY::kKey_F13)   },
        { "KEY_F14", 				EV(KEY::kKey_F14)   },
        { "KEY_F15", 				EV(KEY::kKey_F15)   },
        { "KEY_F16", 				EV(KEY::kKey_F16)   },
        { "KEY_F17", 				EV(KEY::kKey_F17)   },
        { "KEY_F18", 				EV(KEY::kKey_F18)   },
        { "KEY_F19", 				EV(KEY::kKey_F19)   },
        { "KEY_F20", 				EV(KEY::kKey_F20)   },
        { "KEY_F21", 				EV(KEY::kKey_F21)   },
        { "KEY_F22", 				EV(KEY::kKey_F22)   },
        { "KEY_F23", 				EV(KEY::kKey_F23)   },
        { "KEY_F24", 				EV(KEY::kKey_F24)   },
        { "KEY_F25", 				EV(KEY::kKey_F25)   },
        { "KEY_KP_0", 				EV(KEY::kKey_KP_0)  },
        { "KEY_KP_1", 				EV(KEY::kKey_KP_1)  },
        { "KEY_KP_2", 				EV(KEY::kKey_KP_2)  },
        { "KEY_KP_3", 				EV(KEY::kKey_KP_3)  },
        { "KEY_KP_4", 				EV(KEY::kKey_KP_4)  },
        { "KEY_KP_5", 				EV(KEY::kKey_KP_5)  },
        { "KEY_KP_6", 				EV(KEY::kKey_KP_6)  },
        { "KEY_KP_7", 				EV(KEY::kKey_KP_7)  },
        { "KEY_KP_8", 				EV(KEY::kKey_KP_8)  },
        { "KEY_KP_9", 				EV(KEY::kKey_KP_9)  },
        { "KEY_KP_DECIMAL", 		EV(KEY::kKey_KP_DECIMAL)    },
        { "KEY_KP_DIVIDE", 			EV(KEY::kKey_KP_DIVIDE)     },
        { "KEY_KP_MULTIPLY", 		EV(KEY::kKey_KP_MULTIPLY)   },
        { "KEY_KP_SUBTRACT", 		EV(KEY::kKey_KP_SUBTRACT)   },
        { "KEY_KP_ADD", 			EV(KEY::kKey_KP_ADD)        },
        { "KEY_KP_ENTER", 			EV(KEY::kKey_KP_ENTER)      },
        { "KEY_KP_EQUAL", 			EV(KEY::kKey_KP_EQUAL)      },
        { "KEY_LEFT_SHIFT", 		EV(KEY::kKey_LEFT_SHIFT)    },
        { "KEY_LEFT_CONTROL", 		EV(KEY::kKey_LEFT_CONTROL)  },
        { "KEY_LEFT_ALT", 			EV(KEY::kKey_LEFT_ALT)      },
        { "KEY_LEFT_SUPER", 		EV(KEY::kKey_LEFT_SUPER)    },
        { "KEY_RIGHT_SHIFT", 		EV(KEY::kKey_RIGHT_SHIFT)   },
        { "KEY_RIGHT_CONTROL", 		EV(KEY::kKey_RIGHT_CONTROL) },
        { "KEY_RIGHT_ALT", 			EV(KEY::kKey_RIGHT_ALT)     },
        { "KEY_RIGHT_SUPER", 		EV(KEY::kKey_RIGHT_SUPER)   },
        { "KEY_MENU", 				EV(KEY::kKey_MENU)          }
    };

    instance->Set(ctx, binder::to_v8(isolate, "Constants"),
                  binder::to_v8(isolate, constants)).Check();

    instance->Set(ctx, binder::to_v8(isolate, "defaultFontMgr"),
                  binder::Class<CkFontMgr>::create_object(isolate, SkFontMgr::RefDefault())).Check();
}

SkSamplingOptions SamplingToSamplingOptions(int32_t v)
{
    SkSamplingOptions sampling_options;
    switch (v)
    {
    case EV(Sampling::kNearest):
        sampling_options = SkSamplingOptions(SkFilterMode::kNearest);
        break;

    case EV(Sampling::kLinear):
        sampling_options = SkSamplingOptions(SkFilterMode::kLinear);
        break;

    case EV(Sampling::kCubicMitchell):
        sampling_options = SkSamplingOptions(SkCubicResampler::Mitchell());
        break;

    case EV(Sampling::kCubicCatmullRom):
        sampling_options = SkSamplingOptions(SkCubicResampler::CatmullRom());
        break;

    default:
        g_throw(RangeError, "Invalid enumeration value for `sampling`");
    }

    return sampling_options;
}

GALLIUM_BINDINGS_GLAMOR_NS_END
