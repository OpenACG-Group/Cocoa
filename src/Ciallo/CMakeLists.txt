include(FindPkgConfig)

find_package(Vulkan REQUIRED)
pkg_check_modules(XCB xcb xcb-util xcb-xinput REQUIRED)
pkg_check_modules(Cairo cairo REQUIRED)

set(ciallo_target Ciallo)
set(ciallo_sources
        ## Ciallo
        Ciallo.h
        Ciallo.cc
        DrBaseEvent.h
        DrBaseEvent.cc
        DrRepaintEvent.h
        DrButtonPressEvent.h
        DrButtonReleaseEvent.h
        DrMotionEvent.h
        Drawable.h
        Drawable.cc
        DrawableListener.h

        ## XCB platform
        XcbAtom.h
        XcbAtom.cc
        XcbConnection.h
        XcbConnection.cc
        XcbEventQueue.h
        XcbEventQueue.cc
        XcbScreen.h
        XcbScreen.cc
        XcbWindow.h
        XcbWindow.cc

        ## Skia2d backend
        Skia2d/GrContext.h
        Skia2d/GrContext.cc
        Skia2d/GrBaseCompositor.h
        Skia2d/GrBaseCompositor.cc
        Skia2d/GrVulkanCompositor.h
        Skia2d/GrVulkanCompositor.cc
        Skia2d/GrRasterCompositor.h
        Skia2d/GrRasterCompositor.cc
        Skia2d/GrBaseRenderLayer.h
        Skia2d/GrBaseRenderLayer.cc
        Skia2d/GrVulkanRenderLayer.h
        Skia2d/GrVulkanRenderLayer.cc
        Skia2d/GrRasterRenderLayer.h
        Skia2d/GrRasterRenderLayer.cc

        ## Cairo2d backend
        Cairo2d/Cairo2d.h
        Cairo2d/CrSurface.h
        Cairo2d/CrSurface.cc
        Cairo2d/CrDrawableSurface.h
        Cairo2d/CrImageSurface.h
        Cairo2d/CrRecordingSurface.h
        Cairo2d/CrCanvas.h
        Cairo2d/CrCanvas.cc)

add_library(${ciallo_target} STATIC ${ciallo_sources})
target_compile_options(${ciallo_target} PRIVATE ${Cairo_CFLAGS})

target_link_libraries(${ciallo_target}
        PRIVATE
        skia
        Vulkan::Vulkan
        ${XCB_LDFLAGS}
        ${Cairo_LDFLAGS})
