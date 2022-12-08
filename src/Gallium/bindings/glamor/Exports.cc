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

#include "Gallium/bindings/glamor/Exports.h"
#include "Gallium/bindings/glamor/Scene.h"
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
