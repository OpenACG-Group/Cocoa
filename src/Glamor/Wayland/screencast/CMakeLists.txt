find_package(GLIB REQUIRED COMPONENTS gio gobject)
find_package(PipeWire REQUIRED)

add_executable(
        wayland-screencast-bridge
        Screencast.h
        Screencast.cc
        Errors.h
        Errors.cc
        DesktopPortal.h
        DesktopPortal.cc
        Pipewire.h
        Pipewire.cc
        MemoryTexture.h
        MemoryTexture.cc
)

target_include_directories(
        wayland-screencast-bridge
        PRIVATE
        ${GLIB_INCLUDE_DIRS}
        ${PipeWire_INCLUDE_DIRS}
        /usr/include/gio-unix-2.0
)
target_link_libraries(
        wayland-screencast-bridge

        ${GLIB_LIBRARIES}
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES}
        ${PipeWire_LIBRARIES}

        ${LINK_STATIC_FMT}
)

set_target_properties(wayland-screencast-bridge PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
