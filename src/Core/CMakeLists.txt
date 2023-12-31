add_library(
        Core STATIC
        CmdParser.h
        CmdParser.cc
        Journal.h
        Journal.cc
        Exception.h
        Exception.cc
        Utils.h
        Utils.cc
        EventSource.h
        EventLoop.h
        EventLoop.cc
        Filesystem.h
        FilesystemLinuxImpl.cc
        Data.h
        Data.cc
        ScalableWriteBuffer.h
        ScalableWriteBuffer.cc
        Errors.h
        Errors.cc
        EnumClassBitfield.h
        StandaloneThreadPool.h
        StandaloneThreadPool.cc
        ProcessSignalHandler.h
        ProcessSignalHandler.cc
        HuffmanCodec.h
        HuffmanCodec.cc
        ByteArrayCodecs.h
        ByteArrayCodecs.cc
        ConcurrentTaskQueue.h
        ApplicationInfo.h
        ApplicationInfo.cc
        TraceEvent.h
        AsyncMessageQueue.h
        UUIDGenerator.h
        UUIDGenerator.cc
)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(Core
        pthread
        dl
        unwind
        ZLIB::ZLIB
        OpenSSL::SSL
        ${LINK_STATIC_FMT}
        ${LINK_STATIC_LIBUV}
        ${LINK_STATIC_LIBSQUASH})

## For symbol analyzing in RuntimeException
add_link_options(-rdynamic)
