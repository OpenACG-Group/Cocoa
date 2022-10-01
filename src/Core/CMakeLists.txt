set(BINARY_QRES_DIR ${CMAKE_BINARY_DIR}/qresource)
file(MAKE_DIRECTORY ${BINARY_QRES_DIR})
file(MAKE_DIRECTORY ${BINARY_QRES_DIR}/packages)

set(CRPKG_INTERNAL_V8 ${BINARY_QRES_DIR}/packages/org.cocoa.internal.v8.crpkg)

file(GLOB CRPKG_INTERNAL_V8_FILES ${QRESOURCE_DIR}/packages/internal_v8/*)
add_custom_command(
        OUTPUT ${CRPKG_INTERNAL_V8}
        COMMAND ${CMAKE_BINARY_DIR}/cocoa-qresc ${QRESOURCE_DIR}/packages/internal_v8 ${BINARY_QRES_DIR}/packages
        DEPENDS cocoa-qresc ${CRPKG_INTERNAL_V8_FILES}
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
        COMMAND ${CMAKE_HOME_DIRECTORY}/script/intern-crpkgs.py ${CRPKG_INTERNAL_V8}
                >${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
        DEPENDS ${CRPKG_INTERNAL_V8}
)

add_library(
        Core STATIC
        CmdParser.h
        CmdParser.cc
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
        Data.h
        Data.cc
        ScalableWriteBuffer.h
        ScalableWriteBuffer.cc
        Errors.h
        Errors.cc
        EnumClassBitfield.h
        QResource.h
        QResource.cc
        StandaloneThreadPool.h
        StandaloneThreadPool.cc
        ProcessSignalHandler.h
        ProcessSignalHandler.cc
        HuffmanCodec.h
        HuffmanCodec.cc
        ConcurrentTaskQueue.h
        ${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc

        subprocess/SubprocessHost.h
        subprocess/SubprocessHost.cc
        subprocess/HostMessageListener.h
        subprocess/Message.h
        subprocess/Message.cc

        subprocess/SubprocessClient.h
        subprocess/SubprocessClient.cc
)

target_link_libraries(Core
        pthread
        dl
        unwind
        z
        ${LINK_STATIC_FMT}
        ${LINK_STATIC_LIBUV}
        ${LINK_STATIC_LIBSQUASH})

## For symbol analyzing in RuntimeException
add_link_options(-rdynamic)

add_executable(
        ipc-test-subprocess
        subprocess/IpcTestSubprocess.cc
)
set_target_properties(ipc-test-subprocess PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(
        ipc-test-subprocess
        PRIVATE
        Core
)
