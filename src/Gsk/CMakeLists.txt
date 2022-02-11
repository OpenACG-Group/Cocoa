add_library(
        Gsk STATIC

        Gsk.h
        Gsk.cc
        GskEnumerations.h
        GskDisplayManager.h
        GskDisplayManager.cc
        GskDisplay.h
        GskDisplay.cc
        GskEvent.h
        GskEvent.cc
        GskEventQueue.h
        GskEventQueue.cc
        GskMonitor.h
        GskMonitor.cc
        GskContentFormats.h
        GskContentFormats.cc
        GskContentProvider.h
        GskContentProvider.cc
        GskSeat.h
        GskSeat.cc
        GskDevice.h
        GskDevice.cc
        GskKey.h
        GskKeymap.h
        GskKeymap.cc
        GskKeynameTable.h
        GskKeySymbols.h
        GskKeyUnicode.cc
        GskCursor.h
        GskCursor.cc

        wayland/protos/pointer-gestures-unstable-v1-client-protocol.h
        wayland/protos/pointer-gestures-unstable-v1-client-protocol.c
        wayland/protos/primary-selection-unstable-v1-client-protocol.h
        wayland/protos/primary-selection-unstable-v1-client-protocol.c
        wayland/protos/tablet-unstable-v2-client-protocol.h
        wayland/protos/tablet-unstable-v2-client-protocol.c
        wayland/protos/xdg-shell-unstable-v6-protocol.h
        wayland/protos/xdg-shell-unstable-v6-protocol.c
        wayland/protos/xdg-shell-client-protocol.h
        wayland/protos/xdg-shell-client-protocol.c
        wayland/protos/xdg-output-unstable-v1-protocol.h
        wayland/protos/xdg-output-unstable-v1-protocol.c
        wayland/cursor/XCursor.h
        wayland/cursor/XCursor.c
        wayland/cursor/wayland-cursor.h
        wayland/cursor/wayland-cursor.c
        wayland/cursor/os-compatibility.h
        wayland/cursor/os-compatibility.c
        wayland/GskWaylandDisplay.h
        wayland/GskWaylandDisplay.cc
        wayland/GskWaylandMonitor.h
        wayland/GskWaylandMonitor.cc
        wayland/GskWaylandSeat.h
        wayland/GskWaylandSeat.cc
        wayland/GskWaylandKeymap.h
        wayland/GskWaylandKeymap.cc
        wayland/GskWaylandCursor.cc
        wayland/GskWaylandDevice.h
        wayland/GskWaylandDevice.cc
        wayland/GskSkSurfaceWrapper.h
)

find_package(Vulkan REQUIRED)
target_compile_definitions(Gsk PRIVATE SK_VULKAN)
target_link_libraries(
        Gsk
        PRIVATE
        Vulkan::Vulkan
        skia
        PkgConfig::SIGC
        wayland-client
        xkbcommon
        fribidi)

