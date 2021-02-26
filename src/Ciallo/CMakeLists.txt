include(FindPkgConfig)

find_package(Vulkan REQUIRED)
find_package(OpenCL REQUIRED)
pkg_check_modules(XCB xcb xcb-renderutil REQUIRED)

set(ciallo_target Ciallo)
set(ciallo_sources
        GrUtils.h
        GrUtils.cc
        DDR/GrLayerResult.h
        DDR/GrLayerResult.cc
        DDR/GrTargetSurface.h
        DDR/GrTargetSurface.cc
        DDR/GrGpuCompositor.h
        DDR/GrGpuCompositor.cc
        DDR/GrCpuCompositor.h
        DDR/GrCpuCompositor.cc
        DDR/GrOpenCLCompositor.h
        DDR/GrOpenCLCompositor.cc
        DDR/GrBaseCompositor.h
        DDR/GrBaseCompositor.cc
        DDR/GrBaseRenderLayer.h
        DDR/GrBaseRenderLayer.cc
        DDR/GrGpuRenderLayer.h
        DDR/GrGpuRenderLayer.cc
        DDR/GrCpuRenderLayer.h
        DDR/GrCpuRenderLayer.cc
        DDR/GrOpenCLRenderLayer.h
        DDR/GrOpenCLRenderLayer.cc
        DDR/GrBasePlatform.h
        DDR/GrBasePlatform.cc
        DDR/GrXcbPlatform.h
        DDR/GrXcbPlatform.cc
        DIR/BaseNode.h
        DIR/BaseNode.cc
        DIR/CompositeNode.h
        DIR/CompositeNode.cc
        DIR/RenderNode.h
        DIR/RenderNode.cc
        DIR/PaintNode.h
        DIR/PaintNode.cc
        Thread.h
        Thread.cc
        GraphicsContext.h
        GraphicsContext.cc
        BaseWindow.h
        BaseWindow.cc
        XCBWindow.h
        XCBWindow.cc)

add_library(${ciallo_target} STATIC ${ciallo_sources})
target_compile_definitions(${ciallo_target} PRIVATE COCOA_USE_OPENCL=1)

target_link_libraries(${ciallo_target}
        PRIVATE
        skia
        Vulkan::Vulkan
        OpenCL::OpenCL
        ${XCB_LDFLAGS})
