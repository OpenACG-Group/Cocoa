set(target_name Vanilla)

add_library(${target_name} STATIC
            Base.h
            Context.h
            Context.cc
            VaDisplay.h
            VaDisplay.cc
        VaDrawContext.h
            VaWindow.h
            VaWindow.cc
            X11/VaX11Display.h
            X11/VaX11Display.cc
            X11/VaX11Atoms.h
            X11/VaX11Atoms.cc
            X11/VaX11Window.h
            X11/VaX11Window.cc)
target_compile_definitions(${target_name} PRIVATE -DSK_VULKAN)
target_link_libraries(${target_name} PRIVATE
                        skia
                        X11
                        Xrender
                        PkgConfig::SIGC)
