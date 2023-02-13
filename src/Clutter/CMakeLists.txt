find_package(GLIB REQUIRED COMPONENTS gio gobject)
find_package(PipeWire REQUIRED)

add_library(
        Clutter
        STATIC

        Clutter.h
        Clutter.cc
        ProcessHost.h
        ProcessHost.cc
        ProcessListener.h
)

add_library(
        ClutterServiceClient
        STATIC

        ServiceClient/ServiceClient.h
        ServiceClient/ServiceClient.cc
)

add_executable(
        clutter-dummy
        dummy.service/Main.cc
)
target_link_libraries(clutter-dummy PRIVATE ClutterServiceClient ${LINK_STATIC_FMT})
set_target_properties(clutter-dummy PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CLUTTER_EXE_NAME clutter-desktop-integration)
add_executable(
        ${CLUTTER_EXE_NAME}
        desktop-integration.service/Screencast.h
        desktop-integration.service/Screencast.cc
        desktop-integration.service/Errors.h
        desktop-integration.service/Errors.cc
        desktop-integration.service/DesktopPortal.h
        desktop-integration.service/DesktopPortal.cc
        desktop-integration.service/Pipewire.h
        desktop-integration.service/Pipewire.cc
        desktop-integration.service/MemoryTexture.h
        desktop-integration.service/MemoryTexture.cc
)
target_include_directories(
        ${CLUTTER_EXE_NAME}
        PRIVATE
        ${GLIB_INCLUDE_DIRS}
        ${PipeWire_INCLUDE_DIRS}
        /usr/include/gio-unix-2.0
)
target_link_libraries(
        ${CLUTTER_EXE_NAME}

        ClutterServiceClient

        ${GLIB_LIBRARIES}
        ${GLIB_GIO_LIBRARIES}
        ${GLIB_GOBJECT_LIBRARIES}
        ${PipeWire_LIBRARIES}

        ${LINK_STATIC_FMT}
)

set_target_properties(${CLUTTER_EXE_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
