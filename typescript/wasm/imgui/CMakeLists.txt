cmake_minimum_required(VERSION 3.5.0)
project(ImGuiWasmCanvasAPI VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

set(IMGUI_SRC ${CMAKE_SOURCE_DIR}/../../../third_party/imgui)

# include_directories(/usr/lib/emscripten/system/include)

add_executable(
    imgui-canvasapi
    ./imgui_impl_wasm_canvasapi.cc
    ${IMGUI_SRC}/imgui.cpp
    ${IMGUI_SRC}/imgui_demo.cpp
    ${IMGUI_SRC}/imgui_draw.cpp
    ${IMGUI_SRC}/imgui_tables.cpp
    ${IMGUI_SRC}/imgui_widgets.cpp
)

target_include_directories(imgui-canvasapi PRIVATE ${IMGUI_SRC})

target_link_options(
    imgui-canvasapi PRIVATE

    "-lembind"
    "-sEXPORTED_FUNCTIONS=['_malloc', '_free']"
    # "-sEXPORT_ES6=1"
    # "-sMODULARIZE=1"
    "-sNO_DISABLE_EXCEPTION_CATCHING"
    "--pre-js" ${CMAKE_HOME_DIRECTORY}/../helper/heap-memory.js
)
