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

#include "Gallium/bindings/present/PresentThread.h"
#include "Gallium/bindings/present/Display.h"
#include "Gallium/bindings/present/Monitor.h"
#include "Gallium/bindings/present/Cursor.h"
#include "Gallium/bindings/present/Surface.h"
#include "Gallium/bindings/present/ContentAggregator.h"
#include "Gallium/bindings/present/Scene.h"

#include "Gallium/bindings/renderer/Path.h"
#include "Gallium/bindings/renderer/ImageFilter.h"
#include "Gallium/bindings/renderer/Picture.h"

#include "Gallium/ffi/DefineClass.h"
#include "Gallium/ffi/Enum.h"
#include "Gallium/bindings/present/Module.h"
GALLIUM_BINDINGS_NS_BEGIN

PresentModule::PresentModule()
    : ffi::NativeModule("present", "Window system integration and window presentation", {"renderer", "multimedia"})
{
}

ffi::LocalExports PresentModule::OnBuildExports(v8::Isolate *isolate)
{
    using namespace present;
    v8::Local<v8::Context> context = isolate->GetCurrentContext();

    ffi::LocalExports exports;

    //! TSDecl: @enum MonitorSubpixel
    exports["MonitorSubpixel"] = ffi::DefineEnum<gl::MonitorSubpixel>(isolate, "MonitorSubpixel", false)
        //! TSDecl: @enumitem Unknown
        .Item("Unknown", gl::MonitorSubpixel::kUnknown)
        //! TSDecl: @enumitem None
        .Item("None", gl::MonitorSubpixel::kNone)
        //! TSDecl: @enumitem HorizontalRGB
        .Item("HorizontalRGB", gl::MonitorSubpixel::kHorizontalRGB)
        //! TSDecl: @enumitem HorizontalBGR
        .Item("HorizontalBGR", gl::MonitorSubpixel::kHorizontalBGR)
        //! TSDecl: @enumitem VerticalRGB
        .Item("VerticalRGB", gl::MonitorSubpixel::kVerticalRGB)
        //! TSDecl: @enumitem VerticalBGR
        .Item("VerticalBGR", gl::MonitorSubpixel::kVerticalBGR)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum MonitorTransform
    exports["MonitorTransform"] = ffi::DefineEnum<gl::MonitorTransform>(isolate, "MonitorTransform", false)
        //! TSDecl: @enumitem Normal
        .Item("Normal", gl::MonitorTransform::kNormal)
        //! TSDecl: @enumitem Rotate90
        .Item("Rotate90", gl::MonitorTransform::kRotate90)
        //! TSDecl: @enumitem Rotate180
        .Item("Rotate180", gl::MonitorTransform::kRotate180)
        //! TSDecl: @enumitem Rotate270
        .Item("Rotate270", gl::MonitorTransform::kRotate270)
        //! TSDecl: @enumitem Flipped
        .Item("Flipped", gl::MonitorTransform::kFlipped)
        //! TSDecl: @enumitem Flipped90
        .Item("Flipped90", gl::MonitorTransform::kFlipped90)
        //! TSDecl: @enumitem Flipped180
        .Item("Flipped180", gl::MonitorTransform::kFlipped180)
        //! TSDecl: @enumitem Flipped270
        .Item("Flipped270", gl::MonitorTransform::kFlipped270)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum MonitorMode
    exports["MonitorMode"] = ffi::DefineEnum<gl::MonitorMode>(isolate, "MonitorMode", true)
        //! TSDecl: @enumitem Current
        .Item("Current", gl::MonitorMode::kCurrent)
        //! TSDecl: @enumitem Preferred
        .Item("Preferred", gl::MonitorMode::kPreferred)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum PointerButton
    exports["PointerButton"] = ffi::DefineEnum<gl::PointerButton>(isolate, "PointerButton", false)
        //! TSDecl: @enumitem Left
        .Item("Left", gl::PointerButton::kLeft)
        //! TSDecl: @enumitem Right
        .Item("Right", gl::PointerButton::kRight)
        //! TSDecl: @enumitem Middle
        .Item("Middle", gl::PointerButton::kMiddle)
        //! TSDecl: @enumitem Side
        .Item("Side", gl::PointerButton::kSide)
        //! TSDecl: @enumitem Extra
        .Item("Extra", gl::PointerButton::kExtra)
        //! TSDecl: @enumitem Forward
        .Item("Forward", gl::PointerButton::kForward)
        //! TSDecl: @enumitem Back
        .Item("Back", gl::PointerButton::kBack)
        //! TSDecl: @enumitem Task
        .Item("Task", gl::PointerButton::kTask)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum AxisSourceType
    exports["AxisSourceType"] = ffi::DefineEnum<gl::AxisSourceType>(isolate, "AxisSourceType", false)
        //! TSDecl: @enumitem Wheel
        .Item("Wheel", gl::AxisSourceType::kWheel)
        //! TSDecl: @enumitem WheelTilt
        .Item("WheelTilt", gl::AxisSourceType::kWheelTilt)
        //! TSDecl: @enumitem Finger
        .Item("Finger", gl::AxisSourceType::kFinger)
        //! TSDecl: @enumitem Continuous
        .Item("Continuous", gl::AxisSourceType::kContinuous)
        //! TSDecl: @enumitem Unknown
        .Item("Unknown", gl::AxisSourceType::kUnknown)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum KeyboardModifiers
    exports["KeyboardModifiers"] = ffi::DefineEnum<gl::KeyboardModifiers>(isolate, "KeyboardModifiers", true)
        //! TSDecl: @enumitem Control
        .Item("Control", gl::KeyboardModifiers::kControl)
        //! TSDecl: @enumitem Alt
        .Item("Alt", gl::KeyboardModifiers::kAlt)
        //! TSDecl: @enumitem Shift
        .Item("Shift", gl::KeyboardModifiers::kShift)
        //! TSDecl: @enumitem Super
        .Item("Super", gl::KeyboardModifiers::kSuper)
        //! TSDecl: @enumitem CapsLock
        .Item("CapsLock", gl::KeyboardModifiers::kCapsLock)
        //! TSDecl: @enumitem NumLock
        .Item("NumLock", gl::KeyboardModifiers::kNumLock)
        //! TSDecl: @enumitem Meta
        .Item("Meta", gl::KeyboardModifiers::kMeta)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum KeyboardKey
    exports["KeyboardKey"] = ffi::DefineEnum<gl::KeyboardKey>(isolate, "KeyboardKey", false)
        //! TSDecl: @enumitem Placeholder
        .Item("Placeholder", gl::KeyboardKey::kPlaceholder)
        //! TSDecl: @enumitem Key_SPACE
        .Item("Key_SPACE", gl::KeyboardKey::kKey_SPACE)
        //! TSDecl: @enumitem Key_APOSTROPHE
        .Item("Key_APOSTROPHE", gl::KeyboardKey::kKey_APOSTROPHE)
        //! TSDecl: @enumitem Key_COMMA
        .Item("Key_COMMA", gl::KeyboardKey::kKey_COMMA)
        //! TSDecl: @enumitem Key_MINUS
        .Item("Key_MINUS", gl::KeyboardKey::kKey_MINUS)
        //! TSDecl: @enumitem Key_PERIOD
        .Item("Key_PERIOD", gl::KeyboardKey::kKey_PERIOD)
        //! TSDecl: @enumitem Key_SLASH
        .Item("Key_SLASH", gl::KeyboardKey::kKey_SLASH)
        //! TSDecl: @enumitem Key_0
        .Item("Key_0", gl::KeyboardKey::kKey_0)
        //! TSDecl: @enumitem Key_1
        .Item("Key_1", gl::KeyboardKey::kKey_1)
        //! TSDecl: @enumitem Key_2
        .Item("Key_2", gl::KeyboardKey::kKey_2)
        //! TSDecl: @enumitem Key_3
        .Item("Key_3", gl::KeyboardKey::kKey_3)
        //! TSDecl: @enumitem Key_4
        .Item("Key_4", gl::KeyboardKey::kKey_4)
        //! TSDecl: @enumitem Key_5
        .Item("Key_5", gl::KeyboardKey::kKey_5)
        //! TSDecl: @enumitem Key_6
        .Item("Key_6", gl::KeyboardKey::kKey_6)
        //! TSDecl: @enumitem Key_7
        .Item("Key_7", gl::KeyboardKey::kKey_7)
        //! TSDecl: @enumitem Key_8
        .Item("Key_8", gl::KeyboardKey::kKey_8)
        //! TSDecl: @enumitem Key_9
        .Item("Key_9", gl::KeyboardKey::kKey_9)
        //! TSDecl: @enumitem Key_SEMICOLON
        .Item("Key_SEMICOLON", gl::KeyboardKey::kKey_SEMICOLON)
        //! TSDecl: @enumitem Key_EQUAL
        .Item("Key_EQUAL", gl::KeyboardKey::kKey_EQUAL)
        //! TSDecl: @enumitem Key_A
        .Item("Key_A", gl::KeyboardKey::kKey_A)
        //! TSDecl: @enumitem Key_B
        .Item("Key_B", gl::KeyboardKey::kKey_B)
        //! TSDecl: @enumitem Key_C
        .Item("Key_C", gl::KeyboardKey::kKey_C)
        //! TSDecl: @enumitem Key_D
        .Item("Key_D", gl::KeyboardKey::kKey_D)
        //! TSDecl: @enumitem Key_E
        .Item("Key_E", gl::KeyboardKey::kKey_E)
        //! TSDecl: @enumitem Key_F
        .Item("Key_F", gl::KeyboardKey::kKey_F)
        //! TSDecl: @enumitem Key_G
        .Item("Key_G", gl::KeyboardKey::kKey_G)
        //! TSDecl: @enumitem Key_H
        .Item("Key_H", gl::KeyboardKey::kKey_H)
        //! TSDecl: @enumitem Key_I
        .Item("Key_I", gl::KeyboardKey::kKey_I)
        //! TSDecl: @enumitem Key_J
        .Item("Key_J", gl::KeyboardKey::kKey_J)
        //! TSDecl: @enumitem Key_K
        .Item("Key_K", gl::KeyboardKey::kKey_K)
        //! TSDecl: @enumitem Key_L
        .Item("Key_L", gl::KeyboardKey::kKey_L)
        //! TSDecl: @enumitem Key_M
        .Item("Key_M", gl::KeyboardKey::kKey_M)
        //! TSDecl: @enumitem Key_N
        .Item("Key_N", gl::KeyboardKey::kKey_N)
        //! TSDecl: @enumitem Key_O
        .Item("Key_O", gl::KeyboardKey::kKey_O)
        //! TSDecl: @enumitem Key_P
        .Item("Key_P", gl::KeyboardKey::kKey_P)
        //! TSDecl: @enumitem Key_Q
        .Item("Key_Q", gl::KeyboardKey::kKey_Q)
        //! TSDecl: @enumitem Key_R
        .Item("Key_R", gl::KeyboardKey::kKey_R)
        //! TSDecl: @enumitem Key_S
        .Item("Key_S", gl::KeyboardKey::kKey_S)
        //! TSDecl: @enumitem Key_T
        .Item("Key_T", gl::KeyboardKey::kKey_T)
        //! TSDecl: @enumitem Key_U
        .Item("Key_U", gl::KeyboardKey::kKey_U)
        //! TSDecl: @enumitem Key_V
        .Item("Key_V", gl::KeyboardKey::kKey_V)
        //! TSDecl: @enumitem Key_W
        .Item("Key_W", gl::KeyboardKey::kKey_W)
        //! TSDecl: @enumitem Key_X
        .Item("Key_X", gl::KeyboardKey::kKey_X)
        //! TSDecl: @enumitem Key_Y
        .Item("Key_Y", gl::KeyboardKey::kKey_Y)
        //! TSDecl: @enumitem Key_Z
        .Item("Key_Z", gl::KeyboardKey::kKey_Z)
        //! TSDecl: @enumitem Key_LEFT_BRACKET
        .Item("Key_LEFT_BRACKET", gl::KeyboardKey::kKey_LEFT_BRACKET)
        //! TSDecl: @enumitem Key_BACKSLASH
        .Item("Key_BACKSLASH", gl::KeyboardKey::kKey_BACKSLASH)
        //! TSDecl: @enumitem Key_RIGHT_BRACKET
        .Item("Key_RIGHT_BRACKET", gl::KeyboardKey::kKey_RIGHT_BRACKET)
        //! TSDecl: @enumitem Key_GRAVE_ACCENT
        .Item("Key_GRAVE_ACCENT", gl::KeyboardKey::kKey_GRAVE_ACCENT)
        //! TSDecl: @enumitem Key_WORLD_1
        .Item("Key_WORLD_1", gl::KeyboardKey::kKey_WORLD_1)
        //! TSDecl: @enumitem Key_WORLD_2
        .Item("Key_WORLD_2", gl::KeyboardKey::kKey_WORLD_2)
        //! TSDecl: @enumitem Key_ESCAPE
        .Item("Key_ESCAPE", gl::KeyboardKey::kKey_ESCAPE)
        //! TSDecl: @enumitem Key_ENTER
        .Item("Key_ENTER", gl::KeyboardKey::kKey_ENTER)
        //! TSDecl: @enumitem Key_TAB
        .Item("Key_TAB", gl::KeyboardKey::kKey_TAB)
        //! TSDecl: @enumitem Key_BACKSPACE
        .Item("Key_BACKSPACE", gl::KeyboardKey::kKey_BACKSPACE)
        //! TSDecl: @enumitem Key_INSERT
        .Item("Key_INSERT", gl::KeyboardKey::kKey_INSERT)
        //! TSDecl: @enumitem Key_DELETE
        .Item("Key_DELETE", gl::KeyboardKey::kKey_DELETE)
        //! TSDecl: @enumitem Key_RIGHT
        .Item("Key_RIGHT", gl::KeyboardKey::kKey_RIGHT)
        //! TSDecl: @enumitem Key_LEFT
        .Item("Key_LEFT", gl::KeyboardKey::kKey_LEFT)
        //! TSDecl: @enumitem Key_DOWN
        .Item("Key_DOWN", gl::KeyboardKey::kKey_DOWN)
        //! TSDecl: @enumitem Key_UP
        .Item("Key_UP", gl::KeyboardKey::kKey_UP)
        //! TSDecl: @enumitem Key_PAGE_UP
        .Item("Key_PAGE_UP", gl::KeyboardKey::kKey_PAGE_UP)
        //! TSDecl: @enumitem Key_PAGE_DOWN
        .Item("Key_PAGE_DOWN", gl::KeyboardKey::kKey_PAGE_DOWN)
        //! TSDecl: @enumitem Key_HOME
        .Item("Key_HOME", gl::KeyboardKey::kKey_HOME)
        //! TSDecl: @enumitem Key_END
        .Item("Key_END", gl::KeyboardKey::kKey_END)
        //! TSDecl: @enumitem Key_CAPS_LOCK
        .Item("Key_CAPS_LOCK", gl::KeyboardKey::kKey_CAPS_LOCK)
        //! TSDecl: @enumitem Key_SCROLL_LOCK
        .Item("Key_SCROLL_LOCK", gl::KeyboardKey::kKey_SCROLL_LOCK)
        //! TSDecl: @enumitem Key_NUM_LOCK
        .Item("Key_NUM_LOCK", gl::KeyboardKey::kKey_NUM_LOCK)
        //! TSDecl: @enumitem Key_PRINT_SCREEN
        .Item("Key_PRINT_SCREEN", gl::KeyboardKey::kKey_PRINT_SCREEN)
        //! TSDecl: @enumitem Key_PAUSE
        .Item("Key_PAUSE", gl::KeyboardKey::kKey_PAUSE)
        //! TSDecl: @enumitem Key_F1
        .Item("Key_F1", gl::KeyboardKey::kKey_F1)
        //! TSDecl: @enumitem Key_F2
        .Item("Key_F2", gl::KeyboardKey::kKey_F2)
        //! TSDecl: @enumitem Key_F3
        .Item("Key_F3", gl::KeyboardKey::kKey_F3)
        //! TSDecl: @enumitem Key_F4
        .Item("Key_F4", gl::KeyboardKey::kKey_F4)
        //! TSDecl: @enumitem Key_F5
        .Item("Key_F5", gl::KeyboardKey::kKey_F5)
        //! TSDecl: @enumitem Key_F6
        .Item("Key_F6", gl::KeyboardKey::kKey_F6)
        //! TSDecl: @enumitem Key_F7
        .Item("Key_F7", gl::KeyboardKey::kKey_F7)
        //! TSDecl: @enumitem Key_F8
        .Item("Key_F8", gl::KeyboardKey::kKey_F8)
        //! TSDecl: @enumitem Key_F9
        .Item("Key_F9", gl::KeyboardKey::kKey_F9)
        //! TSDecl: @enumitem Key_F10
        .Item("Key_F10", gl::KeyboardKey::kKey_F10)
        //! TSDecl: @enumitem Key_F11
        .Item("Key_F11", gl::KeyboardKey::kKey_F11)
        //! TSDecl: @enumitem Key_F12
        .Item("Key_F12", gl::KeyboardKey::kKey_F12)
        //! TSDecl: @enumitem Key_F13
        .Item("Key_F13", gl::KeyboardKey::kKey_F13)
        //! TSDecl: @enumitem Key_F14
        .Item("Key_F14", gl::KeyboardKey::kKey_F14)
        //! TSDecl: @enumitem Key_F15
        .Item("Key_F15", gl::KeyboardKey::kKey_F15)
        //! TSDecl: @enumitem Key_F16
        .Item("Key_F16", gl::KeyboardKey::kKey_F16)
        //! TSDecl: @enumitem Key_F17
        .Item("Key_F17", gl::KeyboardKey::kKey_F17)
        //! TSDecl: @enumitem Key_F18
        .Item("Key_F18", gl::KeyboardKey::kKey_F18)
        //! TSDecl: @enumitem Key_F19
        .Item("Key_F19", gl::KeyboardKey::kKey_F19)
        //! TSDecl: @enumitem Key_F20
        .Item("Key_F20", gl::KeyboardKey::kKey_F20)
        //! TSDecl: @enumitem Key_F21
        .Item("Key_F21", gl::KeyboardKey::kKey_F21)
        //! TSDecl: @enumitem Key_F22
        .Item("Key_F22", gl::KeyboardKey::kKey_F22)
        //! TSDecl: @enumitem Key_F23
        .Item("Key_F23", gl::KeyboardKey::kKey_F23)
        //! TSDecl: @enumitem Key_F24
        .Item("Key_F24", gl::KeyboardKey::kKey_F24)
        //! TSDecl: @enumitem Key_F25
        .Item("Key_F25", gl::KeyboardKey::kKey_F25)
        //! TSDecl: @enumitem Key_KP_0
        .Item("Key_KP_0", gl::KeyboardKey::kKey_KP_0)
        //! TSDecl: @enumitem Key_KP_1
        .Item("Key_KP_1", gl::KeyboardKey::kKey_KP_1)
        //! TSDecl: @enumitem Key_KP_2
        .Item("Key_KP_2", gl::KeyboardKey::kKey_KP_2)
        //! TSDecl: @enumitem Key_KP_3
        .Item("Key_KP_3", gl::KeyboardKey::kKey_KP_3)
        //! TSDecl: @enumitem Key_KP_4
        .Item("Key_KP_4", gl::KeyboardKey::kKey_KP_4)
        //! TSDecl: @enumitem Key_KP_5
        .Item("Key_KP_5", gl::KeyboardKey::kKey_KP_5)
        //! TSDecl: @enumitem Key_KP_6
        .Item("Key_KP_6", gl::KeyboardKey::kKey_KP_6)
        //! TSDecl: @enumitem Key_KP_7
        .Item("Key_KP_7", gl::KeyboardKey::kKey_KP_7)
        //! TSDecl: @enumitem Key_KP_8
        .Item("Key_KP_8", gl::KeyboardKey::kKey_KP_8)
        //! TSDecl: @enumitem Key_KP_9
        .Item("Key_KP_9", gl::KeyboardKey::kKey_KP_9)
        //! TSDecl: @enumitem Key_KP_DECIMAL
        .Item("Key_KP_DECIMAL", gl::KeyboardKey::kKey_KP_DECIMAL)
        //! TSDecl: @enumitem Key_KP_DIVIDE
        .Item("Key_KP_DIVIDE", gl::KeyboardKey::kKey_KP_DIVIDE)
        //! TSDecl: @enumitem Key_KP_MULTIPLY
        .Item("Key_KP_MULTIPLY", gl::KeyboardKey::kKey_KP_MULTIPLY)
        //! TSDecl: @enumitem Key_KP_SUBTRACT
        .Item("Key_KP_SUBTRACT", gl::KeyboardKey::kKey_KP_SUBTRACT)
        //! TSDecl: @enumitem Key_KP_ADD
        .Item("Key_KP_ADD", gl::KeyboardKey::kKey_KP_ADD)
        //! TSDecl: @enumitem Key_KP_ENTER
        .Item("Key_KP_ENTER", gl::KeyboardKey::kKey_KP_ENTER)
        //! TSDecl: @enumitem Key_KP_EQUAL
        .Item("Key_KP_EQUAL", gl::KeyboardKey::kKey_KP_EQUAL)
        //! TSDecl: @enumitem Key_LEFT_SHIFT
        .Item("Key_LEFT_SHIFT", gl::KeyboardKey::kKey_LEFT_SHIFT)
        //! TSDecl: @enumitem Key_LEFT_CONTROL
        .Item("Key_LEFT_CONTROL", gl::KeyboardKey::kKey_LEFT_CONTROL)
        //! TSDecl: @enumitem Key_LEFT_ALT
        .Item("Key_LEFT_ALT", gl::KeyboardKey::kKey_LEFT_ALT)
        //! TSDecl: @enumitem Key_LEFT_SUPER
        .Item("Key_LEFT_SUPER", gl::KeyboardKey::kKey_LEFT_SUPER)
        //! TSDecl: @enumitem Key_RIGHT_SHIFT
        .Item("Key_RIGHT_SHIFT", gl::KeyboardKey::kKey_RIGHT_SHIFT)
        //! TSDecl: @enumitem Key_RIGHT_CONTROL
        .Item("Key_RIGHT_CONTROL", gl::KeyboardKey::kKey_RIGHT_CONTROL)
        //! TSDecl: @enumitem Key_RIGHT_ALT
        .Item("Key_RIGHT_ALT", gl::KeyboardKey::kKey_RIGHT_ALT)
        //! TSDecl: @enumitem Key_RIGHT_SUPER
        .Item("Key_RIGHT_SUPER", gl::KeyboardKey::kKey_RIGHT_SUPER)
        //! TSDecl: @enumitem Key_MENU
        .Item("Key_MENU", gl::KeyboardKey::kKey_MENU)
        .Finalize();
    //! TSDecl: @end

    //! TSDecl: @enum UpdateResult
    exports["UpdateResult"] = ffi::DefineEnum<gl::ContentAggregator::UpdateResult>(isolate, "UpdateResult", true)
        //! TSDecl: @enumitem Success
        .Item("Success", gl::ContentAggregator::UpdateResult::kSuccess)
        //! TSDecl: @enumitem FrameDropped
        .Item("FrameDropped", gl::ContentAggregator::UpdateResult::kFrameDropped)
        //! TSDecl: @enumitem Error
        .Item("Error", gl::ContentAggregator::UpdateResult::kError)
        .Finalize();
    //! TSDecl: @end

    exports["VideoFrameViewResampler"] = ffi::DefineEnum<VideoFrameViewResampler>(
        isolate, "VideoFrameViewResampler", true)
        .Item("Nearest", VideoFrameViewResampler::kNearest)
        .Item("Bilinear", VideoFrameViewResampler::kBilinear)
        .Item("Bicubic", VideoFrameViewResampler::kBicubic)
        .Finalize();

    ffi::DefineInterface<SurfaceCreationOptions>(isolate, "SurfaceCreationOptions")
        .Field("enableGpuPipeline", &SurfaceCreationOptions::enable_gpu_pipeline)
        .Field("enableGpuVideoDecodeCompatible", &SurfaceCreationOptions::enable_gpu_video_decode_compatible)
        .Finalize();


    exports["PresentThread"] = ffi::DefineClass<PresentThread>(isolate)
        .MethodStatic("Start", PresentThread::Start)
        .Method("dispose", &PresentThread::dispose)
        .Method("collect", &PresentThread::collect)
        .Method("traceResourcesJSON", &PresentThread::traceResourcesJSON)
        .Method("createDisplay", &PresentThread::createDisplay)
        .Finalize(context);

    exports["Display"] = ffi::DefineClass<Display, EventEmitterBase>(isolate)
        .Method("close", &Display::close)
        .Method("requestMonitorList", &Display::requestMonitorList)
        .Property<v8::Local<v8::Value>>("defaultCursorTheme", &Display::getDefaultCursorTheme, nullptr)
        .Method("loadCursorTheme", &Display::loadCursorTheme)
        .Method("createCursor", &Display::createCursor)
        .Method("createSurface", &Display::createSurface)
        .Finalize(context);

    exports["Monitor"] = ffi::DefineClass<Monitor, EventEmitterBase>(isolate)
        .Method("requestPropertySet", &Monitor::requestPropertySet)
        .Finalize(context);

    exports["CursorTheme"] = ffi::DefineClass<CursorTheme>(isolate)
        .Method("dispose", &CursorTheme::dispose)
        .Method("loadCursor", &CursorTheme::loadCursor)
        .Finalize(context);

    exports["Cursor"] = ffi::DefineClass<Cursor>(isolate)
        .Method("dispose", &Cursor::dispose)
        .Method("getHotspotVector", &Cursor::getHotspotVector)
        .Finalize(context);

    exports["Surface"] = ffi::DefineClass<Surface, EventEmitterBase>(isolate)
        .Property<v8::Local<v8::Value>>("dimensions", &Surface::getDimensions, nullptr)
        .Property<v8::Local<v8::Value>>("display", &Surface::getDisplay, nullptr)
        .Property<v8::Local<v8::Value>>("contentAggregator", &Surface::getContentAggregator, nullptr)
        .Method("close", &Surface::close)
        .Method("setTitle", &Surface::setTitle)
        .Method("resize", &Surface::resize)
        .Method("requestBufferStateInfo", &Surface::requestBufferStateInfo)
        .Method("requestNextFrame", &Surface::requestNextFrame)
        .Method("setMaxSize", &Surface::setMaxSize)
        .Method("setMinSize", &Surface::setMinSize)
        .Method("setMaximized", &Surface::setMaximized)
        .Method("setMinimized", &Surface::setMinimized)
        .Method("setFullscreen", &Surface::setFullscreen)
        .Method("setAttachedCursor", &Surface::setAttachedCursor)
        .Method("getVideoDecodeCompatibleDevice", &Surface::getVideoDecodeCompatibleDevice)
        .Finalize(context);

    exports["ContentAggregator"] = ffi::DefineClass<ContentAggregator, EventEmitterBase>(isolate)
        .Method("requestImageInfo", &ContentAggregator::requestImageInfo)
        .Method("purgeRasterCacheResources", &ContentAggregator::purgeRasterCacheResources)
        .Method("update", &ContentAggregator::update)
        .Finalize(context);

    exports["Scene"] = ffi::DefineClass<Scene>(isolate)
        .Method("toString", &Scene::toString)
        .Finalize(context);

    exports["SceneBuilder"] = ffi::DefineClass<SceneBuilder>(isolate)
        .Constructor<const renderer::RectAdapter&>()
        .Method("build", &SceneBuilder::build)
        .Method("pop", &SceneBuilder::pop)
        .Method("pushOffset", &SceneBuilder::pushOffset)
        .Method("pushTransform", &SceneBuilder::pushTransform)
        .Method("addPicture", &SceneBuilder::addPicture)
        .Method("pushOpacity", &SceneBuilder::pushOpacity)
        .Method("pushImageFilter", &SceneBuilder::pushImageFilter)
        .Method("pushBackdropFilter", &SceneBuilder::pushBackdropFilter)
        .Method("pushRectClip", &SceneBuilder::pushRectClip)
        .Method("pushRRectClip", &SceneBuilder::pushRRectClip)
        .Method("pushPathClip", &SceneBuilder::pushPathClip)
        .Method("addVideoFrameView", &SceneBuilder::addVideoFrameView)
        .Finalize(context);

    return exports;
}

GALLIUM_BINDINGS_NS_END
