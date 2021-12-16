enable_language(C ASM CXX)

set(core_target Core)
set(core_sources
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
        EnumClassBitfield.h)

add_library(${core_target} STATIC ${core_sources})

set(SQUASH_LIBS squash z)
target_link_libraries(${core_target}
        PRIVATE
        pthread
        dl
        unwind
        uv_a
        fmt::fmt
        cpu_features
        ${SQUASH_LIBS})

## For symbol analyzing in RuntimeException
add_link_options(-rdynamic)
