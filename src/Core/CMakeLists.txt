enable_language(C ASM CXX)

set(
        QRESOURCE_AUTOLOAD_IMAGES
        ${QRESOURCE_COMPILED_DIR}/org.cocoa.internal.v8.crpkg
)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
        COMMAND ${CMAKE_HOME_DIRECTORY}/script/intern-crpkgs.py
                ${QRESOURCE_AUTOLOAD_IMAGES} >${CMAKE_CURRENT_BINARY_DIR}/QResourceAutoloadTbl.cc
        DEPENDS ${QRESOURCE_AUTOLOAD_IMAGES}
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
