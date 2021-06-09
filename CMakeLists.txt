cmake_minimum_required(VERSION 3.0.0)
project(Cocoa VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SIGC REQUIRED IMPORTED_TARGET sigc++-2.0)

## Link options and include directories
include_directories(
        src
        third_party/skia
        third_party/v8
        third_party/v8/include
        third_party/llvm/include
        third_party/llvm/out/include
        ${SIGC_INCLUDE_DIRS})

link_directories(third_party/skia/out/Shared
                 third_party/poco/out/lib
                 third_party/v8/out/x64.release
                 third_party/llvm/out/lib
                 third_party/libuv/out
                 ${SIGC_LIBRARY_DIRS})

## To suppress warning from Poco library (Clang only)
if(${CMAKE_CXX_COMPILER} MATCHES "clang")
    add_compile_options("-Wno-implicit-const-int-float-conversion")
endif()

## Compile options and targets
add_compile_definitions(COCOA_PROJECT=1 V8_COMPRESS_POINTERS SK_VULKAN)
add_compile_options(-Wall)

add_subdirectory(src/Core)
#add_subdirectory(src/Ciallo)
#add_subdirectory(src/Komorebi)
#add_subdirectory(src/NaGui)
add_subdirectory(src/Vanilla)
add_subdirectory(src/Scripter)
#add_subdirectory(src/virtfs)
add_executable(Cocoa
               src/Main.cc)

target_link_libraries(Cocoa PRIVATE
                      Vanilla
                      Scripter
                      Core)
