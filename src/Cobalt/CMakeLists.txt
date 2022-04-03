add_library(
        Cobalt STATIC

        Cobalt.h
        Cobalt.cc
        RenderHost.h
        RenderHost.cc
        RenderHostInvocation.h
        RenderHostCallbackInfo.h
        RenderHostCallbackInfo.cc
        RenderHostSlotCallbackInfo.h
        RenderHostSlotCallbackInfo.cc
        RenderHostCreator.h
        RenderHostCreator.cc
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
        Surface.h
        Surface.cc
        Blender.h
        Blender.cc
        HWComposeContext.h
        HWComposeContext.cc
        HWComposeSwapchain.h
        HWComposeSwapchain.cc

        Wayland/protos/xdg-shell-client-protocol.h
        Wayland/protos/xdg-shell-client-protocol.c
        Wayland/protos/kde-server-decoration-protocol.h
        Wayland/protos/kde-server-decoration-protocol.c
        Wayland/protos/xdg-decoration-unstable-protocol.h
        Wayland/protos/xdg-decoration-unstable-protocol.c
        Wayland/WaylandUtils.h
        Wayland/WaylandDisplay.h
        Wayland/WaylandDisplay.cc
        Wayland/WaylandRenderTarget.h
        Wayland/WaylandRenderTarget.cc
        Wayland/WaylandSHMRenderTarget.h
        Wayland/WaylandSHMRenderTarget.cc
        Wayland/WaylandHWComposeRenderTarget.h
        Wayland/WaylandHWComposeRenderTarget.cc
        Wayland/WaylandSurface.h
        Wayland/WaylandSurface.cc
)

add_custom_command(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LapsCompiler/Parser
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/LapsCompiler/Parser/LapsParser.cc
        DEPENDS LapsCompiler/Parser/LapsParser.yy
        COMMAND bison -o LapsParser.cc LapsParser.yy
)

add_custom_command(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/LapsCompiler/Parser
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/LapsCompiler/Parser/LapsScanner.cc
        DEPENDS LapsCompiler/Parser/LapsScanner.ll
        COMMAND flex -o LapsScanner.cc LapsScanner.ll
)

find_package(Vulkan REQUIRED)

target_compile_definitions(
        Cobalt PRIVATE

        SK_VULKAN
)

target_link_libraries(
        Cobalt PRIVATE
        Vulkan::Vulkan
        skia
        wayland-client
        xkbcommon
)
