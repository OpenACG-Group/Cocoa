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

#ifndef COCOA_GLAMOR_TYPES_H
#define COCOA_GLAMOR_TYPES_H

#define GLAMOR_NAMESPACE_BEGIN  namespace cocoa::gl {
#define GLAMOR_NAMESPACE_END    }

GLAMOR_NAMESPACE_BEGIN

enum class PointerButton : uint8_t
{
    /* Mouse buttons */
    kLeft,
    kRight,
    kMiddle,
    kSide,
    kExtra,
    kForward,
    kBack,
    kTask
};

enum class AxisSourceType : uint8_t
{
    kWheel,
    kWheelTilt,
    kFinger,
    kContinuous,
    kUnknown
};

enum class KeyboardModifiers : uint32_t
{
    kControl        = 0x01,
    kAlt            = 0x02,
    kShift          = 0x04,
    kSuper          = 0x08,
    kCapsLock       = 0x10,
    kNumLock        = 0x20,
    kMeta           = 0x40
};

enum class KeyboardKey : int16_t
{
    kPlaceholder           = -1,

    kKey_SPACE             = 32,
    kKey_APOSTROPHE        = 39,  /* ' */
    kKey_COMMA             = 44,  /* , */
    kKey_MINUS             = 45,  /* - */
    kKey_PERIOD            = 46,  /* . */
    kKey_SLASH             = 47,  /* / */
    kKey_0                 = 48,
    kKey_1                 = 49,
    kKey_2                 = 50,
    kKey_3                 = 51,
    kKey_4                 = 52,
    kKey_5                 = 53,
    kKey_6                 = 54,
    kKey_7                 = 55,
    kKey_8                 = 56,
    kKey_9                 = 57,
    kKey_SEMICOLON         = 59,  /* ; */
    kKey_EQUAL             = 61,  /* = */
    kKey_A                 = 65,
    kKey_B                 = 66,
    kKey_C                 = 67,
    kKey_D                 = 68,
    kKey_E                 = 69,
    kKey_F                 = 70,
    kKey_G                 = 71,
    kKey_H                 = 72,
    kKey_I                 = 73,
    kKey_J                 = 74,
    kKey_K                 = 75,
    kKey_L                 = 76,
    kKey_M                 = 77,
    kKey_N                 = 78,
    kKey_O                 = 79,
    kKey_P                 = 80,
    kKey_Q                 = 81,
    kKey_R                 = 82,
    kKey_S                 = 83,
    kKey_T                 = 84,
    kKey_U                 = 85,
    kKey_V                 = 86,
    kKey_W                 = 87,
    kKey_X                 = 88,
    kKey_Y                 = 89,
    kKey_Z                 = 90,
    kKey_LEFT_BRACKET      = 91,  /* [ */
    kKey_BACKSLASH         = 92,  /* \ */
    kKey_RIGHT_BRACKET     = 93,  /* ] */
    kKey_GRAVE_ACCENT      = 96,  /* ` */
    kKey_WORLD_1           = 161, /* non-US #1 */
    kKey_WORLD_2           = 162, /* non-US #2 */
    kKey_ESCAPE            = 256,
    kKey_ENTER             = 257,
    kKey_TAB               = 258,
    kKey_BACKSPACE         = 259,
    kKey_INSERT            = 260,
    kKey_DELETE            = 261,
    kKey_RIGHT             = 262,
    kKey_LEFT              = 263,
    kKey_DOWN              = 264,
    kKey_UP                = 265,
    kKey_PAGE_UP           = 266,
    kKey_PAGE_DOWN         = 267,
    kKey_HOME              = 268,
    kKey_END               = 269,
    kKey_CAPS_LOCK         = 280,
    kKey_SCROLL_LOCK       = 281,
    kKey_NUM_LOCK          = 282,
    kKey_PRINT_SCREEN      = 283,
    kKey_PAUSE             = 284,
    kKey_F1                = 290,
    kKey_F2                = 291,
    kKey_F3                = 292,
    kKey_F4                = 293,
    kKey_F5                = 294,
    kKey_F6                = 295,
    kKey_F7                = 296,
    kKey_F8                = 297,
    kKey_F9                = 298,
    kKey_F10               = 299,
    kKey_F11               = 300,
    kKey_F12               = 301,
    kKey_F13               = 302,
    kKey_F14               = 303,
    kKey_F15               = 304,
    kKey_F16               = 305,
    kKey_F17               = 306,
    kKey_F18               = 307,
    kKey_F19               = 308,
    kKey_F20               = 309,
    kKey_F21               = 310,
    kKey_F22               = 311,
    kKey_F23               = 312,
    kKey_F24               = 313,
    kKey_F25               = 314,
    kKey_KP_0              = 320,
    kKey_KP_1              = 321,
    kKey_KP_2              = 322,
    kKey_KP_3              = 323,
    kKey_KP_4              = 324,
    kKey_KP_5              = 325,
    kKey_KP_6              = 326,
    kKey_KP_7              = 327,
    kKey_KP_8              = 328,
    kKey_KP_9              = 329,
    kKey_KP_DECIMAL        = 330,
    kKey_KP_DIVIDE         = 331,
    kKey_KP_MULTIPLY       = 332,
    kKey_KP_SUBTRACT       = 333,
    kKey_KP_ADD            = 334,
    kKey_KP_ENTER          = 335,
    kKey_KP_EQUAL          = 336,
    kKey_LEFT_SHIFT        = 340,
    kKey_LEFT_CONTROL      = 341,
    kKey_LEFT_ALT          = 342,
    kKey_LEFT_SUPER        = 343,
    kKey_RIGHT_SHIFT       = 344,
    kKey_RIGHT_CONTROL     = 345,
    kKey_RIGHT_ALT         = 346,
    kKey_RIGHT_SUPER       = 347,
    kKey_MENU              = 348,
    kLast                  = kKey_MENU
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_TYPES_H
