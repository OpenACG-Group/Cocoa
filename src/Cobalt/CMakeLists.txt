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
        Display.h
        Display.cc

        Wayland/protos/xdg-shell-client-protocol.h
        Wayland/protos/xdg-shell-client-protocol.c
        Wayland/WaylandDisplay.h
        Wayland/WaylandDisplay.cc
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
        PkgConfig::SIGC
        wayland-client
        xkbcommon
)
