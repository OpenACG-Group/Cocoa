/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include <cstdint>
#include <emscripten/em_macros.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include <stdexcept>
#include <vector>

#include "imgui.h"
#include "../helper/heap-memory.h"

namespace {

struct JSConfiguration
{
    float width;
    float height;
    float font_size_px;
    emscripten::val ttf_font_data;

    // Callback: (pixels: Uint8Array, w: number, h: number) => number
    // It should return an integer as texture ID, which will be used to refer to
    // the font atlas texture when rendering.
    emscripten::val on_build_font_atlas;
};

void build_font_atlas(const JSConfiguration& cfg)
{
    ImGuiIO& io = ImGui::GetIO();

    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;

    wasmhelper::HeapMemory ttf_font_data(cfg.ttf_font_data);
    io.Fonts->AddFontFromMemoryTTF(ttf_font_data.GetPtr(), ttf_font_data.GetLength(),
                                   cfg.font_size_px, &font_cfg);

    int width, height;
    unsigned char *pixels;
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Alpha8 image uses 1 byte per pixel
    size_t atlas_byte_size = width * height;
    uint32_t texture_id = cfg.on_build_font_atlas(
        emscripten::val(emscripten::memory_view<uint8_t>(atlas_byte_size, pixels)),
        width, height
    ).as<uint32_t>();
    io.Fonts->SetTexID(reinterpret_cast<void*>(texture_id));

    // Clear font data so free the memory
    io.Fonts->ClearTexData();
}

} // namespace anonymous

void CreateContext(const JSConfiguration& cfg)
{
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.BackendPlatformName = "Cocoa OpenACG.Group";
    io.BackendRendererName = "Skia Renderer";

    ImGui::GetStyle().FrameRounding = 3;

    io.DisplaySize = {cfg.width, cfg.height};
    
    build_font_atlas(cfg);

    ImGui::CreateContext();
    ImGui::StyleColorsLight();
}

enum class InputEventType
{
    kMousePos,
    kMouseButtonLeft,
    kMouseButtonRight,
    kMouseWheel
};

template<typename...ArgsT>
void input_event_check_args(const std::vector<emscripten::val>& args)
{
    constexpr size_t kArgsCount = sizeof...(ArgsT);
    if (args.size() != kArgsCount)
        throw std::runtime_error("number of arguments does not match the input event type");
}

void QueueInputEvent(InputEventType event_type, const emscripten::val& args_val)
{
    ImGuiIO& io = ImGui::GetIO();

    auto args = emscripten::vecFromJSArray<emscripten::val>(args_val);

#define CHECK_ARGS(...) input_event_check_args<__VA_ARGS__>(args)

    switch (event_type)
    {
    case InputEventType::kMousePos:
        CHECK_ARGS(float, float);
        io.AddMousePosEvent(args[0].as<float>(), args[1].as<float>());
        break;

    case InputEventType::kMouseButtonLeft:
        CHECK_ARGS(bool);
        io.AddMouseButtonEvent(ImGuiMouseButton_Left, args[0].as<bool>());
        break;

    case InputEventType::kMouseButtonRight:
        CHECK_ARGS(bool);
        io.AddMouseButtonEvent(ImGuiMouseButton_Right, args[0].as<bool>());
        break;
    
    case InputEventType::kMouseWheel:
        CHECK_ARGS(float, float);
        io.AddMouseWheelEvent(args[0].as<float>(), args[1].as<float>());
        break;
    }

#undef CHECK_ARGS
}

uint32_t SwapRB(uint32_t c)
{
    uint32_t A = c >> 24,
             B = (c >> 16) & 0xff,
             G = (c >> 8) & 0xff,
             R = c & 0xff;
    return (A << 24) | (R << 16) | (G << 8) | B;
}

void Render(const emscripten::val& draw_callback)
{
    ImGui::Render();

    // We fetch the most recent data, and convert it so we can render with JS.
    const ImDrawData *draw_data = ImGui::GetDrawData();
    std::vector<float> pos;
    std::vector<float> uv;
    std::vector<uint32_t> colors;

    for (int i = 0; i < draw_data->CmdListsCount; i++)
    {
        const ImDrawList *cmd_list = draw_data->CmdLists[i];
        
        // De-interleave all vertex data
        size_t nb_vertices = cmd_list->VtxBuffer.size();
        pos.clear(); uv.clear(); colors.clear();
        pos.resize(nb_vertices * 2);
        uv.resize(nb_vertices * 2);
        colors.resize(nb_vertices);

        for (int j = 0; j < cmd_list->VtxBuffer.size(); j++)
        {
            const ImDrawVert& vert = cmd_list->VtxBuffer[j];
            pos[j * 2] = vert.pos.x;
            pos[j * 2 + 1] = vert.pos.y;
            uv[j * 2] = vert.uv.x;
            uv[j * 2 + 1] = vert.uv.y;
            // ImGui colors are RGBA, while Skia accepts BGRA color
            colors[j] = SwapRB(vert.col);
        }

        // Finally, draw the vertices
        int index_offset = 0;
        for (int j = 0; j < cmd_list->CmdBuffer.size(); j++)
        {
            const ImDrawCmd *cmd = &cmd_list->CmdBuffer[j];
            if (cmd->UserCallback)
                cmd->UserCallback(cmd_list, cmd);
            else
            {
                emscripten::val js_pos(emscripten::memory_view<float>(pos.size(), pos.data()));
                emscripten::val js_uv(emscripten::memory_view<float>(uv.size(), uv.data()));
                emscripten::val js_colors(emscripten::memory_view<uint32_t>(colors.size(), colors.data()));
                emscripten::val js_indices(emscripten::memory_view<uint16_t>(
                    cmd->ElemCount, cmd_list->IdxBuffer.begin() + index_offset));

                const ImVec4& clip = cmd->ClipRect;
                draw_callback(clip.x, clip.y, clip.z, clip.w, js_pos, js_uv, js_colors, js_indices,
                              reinterpret_cast<uint32_t>(cmd->TextureId));
                
                index_offset += cmd->ElemCount;
            }
        }
    }
}

EMSCRIPTEN_BINDINGS(ImGui)
{
    using namespace emscripten;

    value_object<JSConfiguration>("JSConfiguration")
        .field("width", &JSConfiguration::width)
        .field("height", &JSConfiguration::height)
        .field("fontSizePx", &JSConfiguration::font_size_px)
        .field("ttfFontData", &JSConfiguration::ttf_font_data)
        .field("onBuildFontAtlas", &JSConfiguration::on_build_font_atlas);

    enum_<InputEventType>("InputEventType")
        .value("MousePos", InputEventType::kMousePos)
        .value("MouseButtonLeft", InputEventType::kMouseButtonLeft)
        .value("MouseButtonRight", InputEventType::kMouseButtonRight)
        .value("MouseWheel", InputEventType::kMouseWheel);

    function("Render", Render);
    function("CreateContext", CreateContext);
    function("QueueInputEvent", QueueInputEvent);

    // ImGui exports
    function("NewFrame", ImGui::NewFrame);
    function("ShowDemoWindow", optional_override([](bool closable) {
        if (closable)
        {
            ImGui::ShowDemoWindow(&closable);
            return closable;
        }
        else
        {
            ImGui::ShowDemoWindow();
            return true;
        }
    }));
}
