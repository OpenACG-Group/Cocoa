cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)


## Build options
option(COCOA_BUILD_TOY_NATIVES "Build toy natives for testing purpose" ON)
option(COCOA_BUILD_WITH_ASAN "Build Cocoa with LLVM address sanitizer" OFF)

find_package(PkgConfig REQUIRED)

set(COCOA_THIRDPARTY_DIR ${CMAKE_HOME_DIRECTORY}/third_party)
set(INTERNAL_RESOURCES_DIR ${CMAKE_HOME_DIRECTORY}/internal)

set(CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake)

## Link options and include directories
include_directories(
        src
        /usr/include/pipewire-0.3
        /usr/include/spa-0.2
        third_party/skia
        third_party/v8
        third_party/v8/include
        third_party/build/include)

link_directories(third_party/build/lib)

## Compile options and targets
add_compile_definitions(COCOA_PROJECT=1 V8_COMPRESS_POINTERS SK_VULKAN SK_GANESH SK_DISABLE_LEGACY_SKSURFACE_FLUSH)
add_compile_options(-Wall -fno-rtti)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    ## LLDB cannot inspect std::string object without this compilation option.
    ## See: https://bugs.llvm.org/show_bug.cgi?id=24202
    add_compile_options(-fstandalone-debug)
endif()

add_subdirectory(src/Core)
add_subdirectory(src/CRPKG)
add_subdirectory(src/Glamor)
add_subdirectory(src/Gallium)
# add_subdirectory(src/Reactor)
add_subdirectory(src/Utau)

add_executable(Cocoa src/Main.cc)

set(LINK_STATIC_JSONCPP ${COCOA_THIRDPARTY_DIR}/build/lib/libjsoncpp.a CACHE STRING "Path of static library jsoncpp")
set(LINK_STATIC_JEMALLOC ${COCOA_THIRDPARTY_DIR}/build/lib/libjemalloc_pic.a CACHE STRING "Path of static library jemalloc")
set(LINK_STATIC_FMT ${COCOA_THIRDPARTY_DIR}/build/lib/libfmt.a CACHE STRING "Path of static library fmt")
set(LINK_STATIC_LIBUV ${COCOA_THIRDPARTY_DIR}/build/lib/libuv_a.a CACHE STRING "Path of static library libuv")

target_link_libraries(
        Cocoa
        Gallium
        Utau
        Glamor
        # Reactor
        CRPKG
        Core
        ${LINK_STATIC_JSONCPP}
)

if (${COCOA_BUILD_WITH_ASAN})
    add_compile_options(-fsanitize=address)
    target_link_libraries(Cocoa asan)
else()
    ## We do not use jemalloc when the address sanitizer is enabled.
    target_link_libraries(Cocoa ${LINK_STATIC_JEMALLOC})
endif()

add_library(perfetto STATIC ${COCOA_THIRDPARTY_DIR}/perfetto/sdk/perfetto.cc)
target_link_libraries(Cocoa perfetto)
