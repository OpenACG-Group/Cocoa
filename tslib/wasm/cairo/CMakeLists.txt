cmake_minimum_required(VERSION 3.0.0)
project(CairoWasmIntegration VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(THIRDPARTY_WASM_BUILD ${CMAKE_SOURCE_DIR}/../../../third_party/wasm-build)
set(THIRDPARTY_WASM_INCLUDE_DIR ${THIRDPARTY_WASM_BUILD}/include)
set(THIRDPARTY_WASM_LIBS_DIR ${THIRDPARTY_WASM_BUILD}/lib)

include_directories(/usr/lib/emscripten/system/include)

add_executable(
    CairoBindings
    src/cairo-bindings.cc
)

target_include_directories(
    CairoBindings
    PRIVATE
    ${THIRDPARTY_WASM_INCLUDE_DIR}
    ${THIRDPARTY_WASM_INCLUDE_DIR}/glib-2.0
    ${THIRDPARTY_WASM_INCLUDE_DIR}/glib-2.0/include
    ${THIRDPARTY_WASM_LIBS_DIR}/glib-2.0/include
    ${THIRDPARTY_WASM_INCLUDE_DIR}/cairo
)

target_link_options(
    CairoBindings PRIVATE
    "-lembind"
    "-sEXPORTED_FUNCTIONS=['_malloc', '_free']"
    "-sEXPORT_ES6=1"
    "-sMODULARIZE=1"
    "-sNO_DISABLE_EXCEPTION_CATCHING"
)

target_link_libraries(
    CairoBindings
    PRIVATE
    ${THIRDPARTY_WASM_LIBS_DIR}/libcairo.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libpixman-1.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libfreetype.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libfontconfig.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libpng16.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libharfbuzz.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libz.a
    ${THIRDPARTY_WASM_LIBS_DIR}/libexpat.a
)