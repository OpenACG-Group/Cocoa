cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)

set(BUILD_SHARED_LIBS false)
set(FMT_TEST false)
add_subdirectory(third_party/fmt)
unset(BUILD_SHARED_LIBS)
unset(FMT_TEST)

set(LIBUV_BUILD_TESTS false)
add_subdirectory(third_party/libuv)
unset(LIBUV_BUILD_TESTS)

add_subdirectory(third_party/libsquash)

set(BUILD_TESTING OFF)
add_subdirectory(third_party/cpu_features)
unset(BUILD_TESTING)

set(COCOA_THIRDPARTY_DIR ${CMAKE_HOME_DIRECTORY}/third_party)
set(QRESOURCE_DIR "${CMAKE_HOME_DIRECTORY}/qresource")
set(QRESOURCE_COMPILED_DIR "${QRESOURCE_DIR}/compiled")

## Link options and include directories
include_directories(
        src
        third_party/skia
        third_party/v8
        third_party/v8/include
        third_party/cpu_features
        third_party/fmt/include
        third_party/libsquash/include
        third_party/jsoncpp/include
        third_party/build/include)

link_directories(third_party/build/lib)

if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(squash PRIVATE -Wno-incompatible-pointer-types-discards-qualifiers)
    target_compile_options(uv_a PRIVATE -Wno-sign-compare)
    target_compile_options(uv PRIVATE -Wno-sign-compare)
endif()

## Compile options and targets
add_compile_definitions(COCOA_PROJECT=1 V8_COMPRESS_POINTERS SK_VULKAN)
add_compile_options(-Wall -mrdrnd)

add_subdirectory(src/Core)
add_subdirectory(src/Cobalt)
add_subdirectory(src/Gallium)
add_subdirectory(src/Reactor)
add_subdirectory(qresource/qresourcekit)

add_executable(Cocoa src/Main.cc)

target_link_libraries(
        Cocoa
        Gallium
        Cobalt
        Reactor
        Core
        ${COCOA_THIRDPARTY_DIR}/jsoncpp/out/lib/libjsoncpp.a
        ${COCOA_THIRDPARTY_DIR}/build/lib/libjemalloc_pic.a
)
