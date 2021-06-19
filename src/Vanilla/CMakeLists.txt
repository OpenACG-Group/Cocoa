set(target_name Vanilla)

find_package(Vulkan REQUIRED)
add_library(${target_name} STATIC
            Base.h
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
            Xcb/EventHandlerMacros.h
            Xcb/VaXcbDisplay.h
            Xcb/VaXcbDisplay.cc
            Xcb/VaXcbAtoms.h
            Xcb/VaXcbAtoms.cc
            Xcb/VaXcbWindow.h
            Xcb/VaXcbWindow.cc
            Xcb/VaXcbEventQueue.h
            Xcb/VaXcbEventQueue.cc
            Xcb/VaXcbRasterDrawContext.h
            Xcb/VaXcbRasterDrawContext.cc)
target_compile_definitions(${target_name} PRIVATE SK_VULKAN)
target_link_libraries(${target_name} PRIVATE
                        skia
                        xcb
                        xcb-image
                        xcb-util
                        PkgConfig::SIGC
                        Vulkan::Vulkan)
