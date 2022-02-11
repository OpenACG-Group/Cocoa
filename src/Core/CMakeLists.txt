enable_language(C ASM CXX)

set(
        QRESOURCE_AUTOLOAD_IMAGES
        ${QRESOURCE_COMPILED_DIR}/org.cocoa.internal.v8.crpkg
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
        COMMAND ${CMAKE_HOME_DIRECTORY}/script/intern-crpkgs.py
                ${QRESOURCE_AUTOLOAD_IMAGES}
                >${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
        DEPENDS ${QRESOURCE_AUTOLOAD_IMAGES}
)

add_library(
        Core STATIC
        Journal.h
        Journal.cc
        Exception.h
        Exception.cc
        Properties.h
        Properties.cc
        Utils.h
        Utils.cc
        EventSource.h
        EventLoop.h
        EventLoop.cc
        Filesystem.h
        FilesystemLinuxImpl.cc
        CrpkgImage.h
        CrpkgImage.cc
        CrpkgInputStream.h
        CrpkgInputStream.cc
        Data.h
        Data.cc
        CpuInfo.h
        CpuInfo.cc
        Errors.h
        Errors.cc
        EnumClassBitfield.h
        DBusConnection.h
        DBusConnection.cc
        QResource.h
        QResource.cc
        ${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
)

target_link_libraries(Core
        PRIVATE
        pthread
        dl
        unwind
        uv_a
        fmt::fmt
        cpu_features

        ## DBus libraries
        PkgConfig::SIGC
        dbus-cxx

        ## SquashFS required libraries
        squash z)

## For symbol analyzing in RuntimeException
add_link_options(-rdynamic)
