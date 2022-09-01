add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RenpyMoeOpcodeCodeGen.h
        COMMAND ${CMAKE_HOME_DIRECTORY}/script/renpy-codegen-moe-opcode.py cpp-header
                ${CMAKE_CURRENT_SOURCE_DIR}/Moe/moe-opcode-codegen.renpy > ${CMAKE_CURRENT_BINARY_DIR}/RenpyMoeOpcodeCodeGen.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Moe/moe-opcode-codegen.renpy
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/RenpyMoeOpcodeCodeGen.hpp
        COMMAND ${CMAKE_HOME_DIRECTORY}/script/renpy-codegen-moe-opcode.py cpp-impl
        ${CMAKE_CURRENT_SOURCE_DIR}/Moe/moe-opcode-codegen.renpy > ${CMAKE_CURRENT_BINARY_DIR}/RenpyMoeOpcodeCodeGen.hpp
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Moe/moe-opcode-codegen.renpy
)

add_library(
        Glamor STATIC

        Types.h
        Glamor.h
        Glamor.cc
        ForwardTypeDecls.h
        MaybeGpuObject.h
        MaybeGpuObject.cc
        RenderHost.h
        RenderHost.cc
        RenderHostInvocation.h
        RenderHostCallbackInfo.h
        RenderHostCallbackInfo.cc
        RenderHostSlotCallbackInfo.h
        RenderHostSlotCallbackInfo.cc
        RenderHostCreator.h
        RenderHostCreator.cc
        RenderHostTaskRunner.h
        RenderHostTaskRunner.cc
        RenderClient.h
        RenderClient.cc
        RenderClientObject.h
        RenderClientObject.cc
        RenderClientCallInfo.h
        RenderClientEmitterInfo.h
        RenderClientTransfer.h
        RenderClientSignalEmit.h
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
        HWComposeSwapchain.h
        HWComposeSwapchain.cc
        HWComposeTileFrameGenerator.h
        HWComposeTileFrameGenerator.cc
        GraphicsResourcesTrackable.h
        GraphicsResourcesTrackable.cc
        Cursor.h
        Cursor.cc
        CursorTheme.h
        CursorTheme.cc
        TextureFactory.h
        TextureFactory.cc
        Texture.h
        Texture.cc
        TextureManager.h
        TextureManager.cc

        Layers/RasterCache.h
        Layers/RasterCache.cc
        Layers/LayerTree.h
        Layers/LayerTree.cc
        Layers/RasterDrawOpObserver.h
        Layers/RasterDrawOpObserver.cc
        Layers/Layer.h
        Layers/Layer.cc
        Layers/ContainerLayer.h
        Layers/ContainerLayer.cc
        Layers/PictureLayer.h
        Layers/PictureLayer.cc
        Layers/TransformLayer.h
        Layers/TransformLayer.cc
        Layers/TextureLayer.h
        Layers/TextureLayer.cc

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
        Wayland/WaylandCursorTheme.h
        Wayland/WaylandCursorTheme.cc
        Wayland/WaylandCursor.h
        Wayland/WaylandSystemCursor.h
        Wayland/WaylandSystemCursor.cc
        Wayland/WaylandBitmapCursor.h
        Wayland/WaylandBitmapCursor.cc
        Wayland/WaylandSharedMemoryHelper.h
        Wayland/WaylandSharedMemoryHelper.cc

        Moe/MoeInstructionBase.h
        Moe/MoeHeap.h
        Moe/MoeHeap.cc
        Moe/MoeByteStreamReader.h
        Moe/MoeByteStreamReader.cc
        Moe/MoeCodeHolder.h
        Moe/MoeInterpreterEngine.h
        Moe/MoeInterpreterEngine.cc
        Moe/MoeCodeDisassembler.h
        Moe/MoeCodeDisassembler.cc
        Moe/MoeCodeCompressor.h
        Moe/MoeCodeCompressor.cc
        Moe/MoeOpcodeRenpyInterface.h
        ${CMAKE_CURRENT_BINARY_DIR}/RenpyMoeOpcodeCodeGen.hpp
        ${CMAKE_CURRENT_BINARY_DIR}/RenpyMoeOpcodeCodeGen.h

        GShader/GSAssembly.h
        GShader/GSEnvironment.h
        GShader/GSModule.h
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
        wayland-client
        wayland-cursor
        xkbcommon
)

target_include_directories(
        Glamor PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(Wayland/screencast)
