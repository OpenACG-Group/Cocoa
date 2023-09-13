add_library(
        Glamor STATIC

        Types.h
        Glamor.h
        Glamor.cc
        ForwardTypeDecls.h
        MaybeGpuObject.h
        MaybeGpuObject.cc

        PresentThread.h
        PresentThread.cc
        PresentMessage.h
        PresentRemoteCallMessage.h
        PresentRemoteCall.h
        PresentRemoteCallReturn.h
        PresentRemoteCallReturn.cc
        PresentSignalMessage.h
        PresentSignal.h
        PresentSignalArgs.h
        PresentSignalArgs.cc
        PresentThreadTaskRunner.h
        PresentThreadTaskRunner.cc
        PresentRemoteHandle.h
        PresentRemoteHandle.cc
        RenderTarget.h
        RenderTarget.cc
        Display.h
        Display.cc
        Monitor.h
        Monitor.cc
        FrameNotificationRouter.h
        Surface.h
        Surface.cc
        Blender.h
        Blender.cc
        FrameGeneratorBase.h
        RasterFrameGenerator.h
        RasterFrameGenerator.cc
        HWComposeContext.h
        HWComposeContext.cc
        HWComposeDevice.h
        HWComposeDevice.cc
        HWComposeSwapchain.h
        HWComposeSwapchain.cc
        HWComposeOffscreen.h
        HWComposeOffscreen.cc
        SkiaGpuContextOwner.h
        SkiaGpuContextOwner.cc
        GraphicsResourcesTrackable.h
        GraphicsResourcesTrackable.cc
        Cursor.h
        Cursor.cc
        CursorTheme.h
        CursorTheme.cc
        GProfiler.h
        GProfiler.cc
        SkEventTracerImpl.h
        SkEventTracerImpl.cc

        Layers/RasterCacheKey.h
        Layers/RasterCache.h
        Layers/RasterCache.cc
        Layers/LayerTree.h
        Layers/LayerTree.cc
        Layers/RasterDrawOpObserver.h
        Layers/Layer.h
        Layers/Layer.cc
        Layers/ContainerLayer.h
        Layers/ContainerLayer.cc
        Layers/PictureLayer.h
        Layers/PictureLayer.cc
        Layers/TransformLayer.h
        Layers/TransformLayer.cc
        Layers/ImageFilterLayer.h
        Layers/ImageFilterLayer.cc
        Layers/BackdropFilterLayer.h
        Layers/BackdropFilterLayer.cc
        Layers/ClippingLayerBase.h
        Layers/RectClipLayer.h
        Layers/RectClipLayer.cc
        Layers/RRectClipLayer.h
        Layers/RRectClipLayer.cc
        Layers/PathClipLayer.h
        Layers/PathClipLayer.cc
        Layers/OpacityLayer.h
        Layers/OpacityLayer.cc
        Layers/ExternalTextureLayer.h
        Layers/ExternalTextureLayer.cc

        Wayland/protos/xdg-shell-client-protocol.h
        Wayland/protos/xdg-shell-client-protocol.c
        Wayland/protos/kde-server-decoration-protocol.h
        Wayland/protos/kde-server-decoration-protocol.c
        Wayland/protos/xdg-decoration-unstable-protocol.h
        Wayland/protos/xdg-decoration-unstable-protocol.c
        Wayland/WaylandUtils.h
        Wayland/WaylandDisplay.h
        Wayland/WaylandDisplay.cc
        Wayland/WaylandMonitor.h
        Wayland/WaylandMonitor.cc
        Wayland/WaylandRenderTarget.h
        Wayland/WaylandRenderTarget.cc
        Wayland/WaylandSHMRenderTarget.h
        Wayland/WaylandSHMRenderTarget.cc
        Wayland/WaylandHWComposeRenderTarget.h
        Wayland/WaylandHWComposeRenderTarget.cc
        Wayland/WaylandSurface.h
        Wayland/WaylandSurface.cc
        Wayland/WaylandSeat.h
        Wayland/WaylandSeat.cc
        Wayland/WaylandInputContext.h
        Wayland/WaylandInputContext.cc
        Wayland/WaylandSeatPointerDevice.h
        Wayland/WaylandSeatPointerDevice.cc
        Wayland/WaylandSeatKeyboardDevice.h
        Wayland/WaylandSeatKeyboardDevice.cc
        Wayland/WaylandCursorTheme.h
        Wayland/WaylandCursorTheme.cc
        Wayland/WaylandCursor.h
        Wayland/WaylandSystemCursor.h
        Wayland/WaylandSystemCursor.cc
        Wayland/WaylandBitmapCursor.h
        Wayland/WaylandBitmapCursor.cc
        Wayland/WaylandSharedMemoryHelper.h
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

        ${COCOA_THIRDPARTY_DIR}/build/lib/libskia.a

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

