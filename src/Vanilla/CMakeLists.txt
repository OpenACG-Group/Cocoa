set(target_name Vanilla)

find_package(Vulkan REQUIRED)
find_package(DBus1 REQUIRED)

add_library(${target_name} STATIC
            Base.h
            Base.cc
            Typetraits.h
            Context.h
            Context.cc
            VaDisplay.h
            VaDisplay.cc
            VaDrawContext.h
            VaDrawContext.cc
            VaVkDrawContext.h
            VaVkDrawContext.cc
            VaWindow.h
            VaWindow.cc
            VaKeySymbols.h
            VaKeySymbols.cc
            VaKeyboardProxy.h
            VaKeyboardProxy.cc
            Xcb/EventHandlerMacros.h
            Xcb/VaXcbDisplay.h
            Xcb/VaXcbDisplay.cc
            Xcb/VaXcbDisplayXInput.cc
            Xcb/VaXcbAtoms.h
            Xcb/VaXcbAtoms.cc
            Xcb/VaXcbWindow.h
            Xcb/VaXcbWindow.cc
            Xcb/VaXcbKeyboard.h
            Xcb/VaXcbKeyboard.cc
            Xcb/VaXcbEventQueue.h
            Xcb/VaXcbEventQueue.cc
            Xcb/VaXcbRasterDrawContext.h
            Xcb/VaXcbRasterDrawContext.cc
            Shader/ShaderExecutor.h
            Shader/ShaderExecutor.cc)

target_compile_definitions(${target_name} PRIVATE SK_VULKAN ${DBus1_DEFINITIONS})
target_link_libraries(${target_name} PRIVATE
                        skia
                        xcb
                        xcb-image
                        xcb-util
                        xcb-xkb
                        xcb-xinput
                        xkbcommon
                        xkbcommon-x11
                        PkgConfig::SIGC
                        Vulkan::Vulkan
                        LLVM-12)

add_library(Shader SHARED
            Shader/ShaderLibrary.h
            Shader/ShaderLibrary.cc)
