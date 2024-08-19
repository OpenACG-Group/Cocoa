add_library(
        Glamor STATIC

        Glamor.cc
        MaybeGpuObject.cc
        VulkanAMDAllocatorImpl.cc

        PresentThread.cc
        PresentRemoteCallReturn.cc
        PresentSignalArgs.cc
        PresentThreadTaskRunner.cc
        PresentRemoteHandle.cc
        RenderTarget.cc
        Display.cc
        Monitor.cc
        Surface.cc
        ContentAggregator.cc
        HWComposeContext.cc
        HWComposeDevice.cc
        HWComposeSwapchain.cc
        HWComposeOffscreen.cc
        SkiaGpuContextOwner.cc
        GraphicsResourcesTrackable.cc
        Cursor.cc
        CursorTheme.cc
        SkEventTracerImpl.cc

        Layers/LayerGenerationCache.cc
        Layers/RasterCache.cc
        Layers/LayerTree.cc
        Layers/Layer.cc
        Layers/ContainerLayer.cc
        Layers/PictureLayer.cc
        Layers/TransformLayer.cc
        Layers/ImageFilterLayer.cc
        Layers/BackdropFilterLayer.cc
        Layers/RectClipLayer.cc
        Layers/RRectClipLayer.cc
        Layers/PathClipLayer.cc
        Layers/OpacityLayer.cc
        Layers/ExternalTextureLayer.cc
        Layers/GpuSurfaceViewLayer.cc

        Wayland/protos/xdg-shell-client-protocol.c
        Wayland/protos/kde-server-decoration-protocol.c
        Wayland/protos/xdg-decoration-unstable-protocol.c
        Wayland/protos/viewporter.c
        Wayland/protos/fractional-scale-v1.c
        Wayland/WaylandDisplay.cc
        Wayland/WaylandMonitor.cc
        Wayland/WaylandRenderTarget.cc
        Wayland/WaylandSHMRenderTarget.cc
        Wayland/WaylandHWComposeRenderTarget.cc
        Wayland/WaylandSurface.cc
        Wayland/WaylandSeat.cc
        Wayland/WaylandInputContext.cc
        Wayland/WaylandSeatPointerDevice.cc
        Wayland/WaylandSeatKeyboardDevice.cc
        Wayland/WaylandCursorTheme.cc
        Wayland/WaylandSystemCursor.cc
        Wayland/WaylandBitmapCursor.cc
        Wayland/WaylandSharedMemoryHelper.cc
)

find_package(Vulkan REQUIRED)

target_compile_definitions(
        Glamor PRIVATE

        SK_VULKAN
)

target_link_libraries(
        Glamor PRIVATE
        Vulkan::Vulkan

        skia
        # ${COCOA_THIRDPARTY_DIR}/build/lib/libskcms.a

        ## Skia dependencies
        fontconfig
        freetype
        png16
        jpeg
        webp
        webpdemux
        webpmux

        ## Window system
        wayland-client
        wayland-cursor
        xkbcommon
)

target_include_directories(
        Glamor PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

