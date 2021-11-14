cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SIGC REQUIRED IMPORTED_TARGET sigc++-3.0)

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

## Link options and include directories
include_directories(
        src
        third_party/skia
        third_party/v8
        third_party/v8/include
        third_party/cpu_features
        third_party/fmt/include
        third_party/libsquash/include
        third_party/imgui
        third_party/build/include
        third_party/build/include/dbus-cxx-2.0
        ${SIGC_INCLUDE_DIRS})

link_directories(third_party/skia/out/Shared
                 third_party/poco/out/lib
                 third_party/v8/out/x64.release
                 third_party/build/lib
                 ${SIGC_LIBRARY_DIRS})

## For libsquash
if("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(squash PRIVATE -Wno-incompatible-pointer-types-discards-qualifiers)
endif()

## Compile options and targets
add_compile_definitions(COCOA_PROJECT=1 V8_COMPRESS_POINTERS SK_VULKAN)
add_compile_options(-Wall -mrdrnd)

add_subdirectory(src/Core)
add_subdirectory(src/Vanilla)
add_subdirectory(src/Koi)
add_executable(Cocoa src/Main.cc)

target_link_libraries(Cocoa
        Koi
        Vanilla
        Core)

