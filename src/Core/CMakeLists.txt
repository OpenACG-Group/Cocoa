enable_language(C ASM CXX)

set(core_target Core)
set(core_sources
        Journal.h
        Journal.cc
        Exception.h
        Exception.cc
        PropertyTree.h
        PropertyTree.cc
        StrictJSONParser.h
        StrictJSONParser.cc
        Configurator.h
        Configurator.cc
        Utils.h
        Utils.cc
        PosixSignalCatcher.h
        PosixSignalCatcher.cc
        PosixSigsegvHelper.S
        EventSource.h
        EventLoop.h
        EventLoop.cc
        RandomDevice.h
        RandomDevice.cc)

add_library(${core_target} STATIC ${core_sources})

target_link_libraries(${core_target}
        PRIVATE
        pthread
        dl
        ffi
        unwind
        PocoFoundation
        PocoJSON
        uv_a
        fmt::fmt)

## For symbol analyzing in RuntimeException
add_link_options(-rdynamic)
