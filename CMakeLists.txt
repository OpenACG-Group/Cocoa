cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

## Dependencies
find_package(Poco REQUIRED Foundation JSON)

set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_STATIC ON)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_MULTITHREAD_SUPPORT ON)
add_subdirectory(third_party/zstd/build/cmake)

## Link options and include directories
include_directories(
        src
        third_party/skia
        third_party/v8
        third_party/zstd/lib)

link_directories(third_party/skia/out/Shared)

## To suppress warning from Poco library (Clang only)
if(${CMAKE_CXX_COMPILER} MATCHES "clang")
    add_compile_options("-Wno-implicit-const-int-float-conversion")
endif()

## Compile options and targets
add_compile_definitions(COCOA_PROJECT=1 V8_COMPRESS_POINTERS)
add_compile_options("-Wall")

add_subdirectory(src/Core)
add_subdirectory(src/Ciallo)
add_subdirectory(src/crpkg)

add_compile_options(${Cairo_CFLAGS})
add_executable(Cocoa
               src/Main.cc)

target_link_libraries(Cocoa
        PRIVATE
            Ciallo
            Core
            crpkg)
