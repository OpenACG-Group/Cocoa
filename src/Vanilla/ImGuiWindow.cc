#include "imgui.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSwizzle.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkVertices.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkStream.h"
#include "include/private/SkTDArray.h"

#include "Core/Project.h"
#include "Core/Journal.h"
#include "Vanilla/ImGuiWindow.h"
#include "Vanilla/DrawContext.h"
VANILLA_NS_BEGIN

namespace {
void build_im_font_atlas(ImFontAtlas& atlas, SkPaint& paint)
{
    int w, h;
    uint8_t *pixels;
    atlas.GetTexDataAsAlpha8(&pixels, &w, &h);

    SkImageInfo info = SkImageInfo::MakeA8(w, h);
    SkPixmap pixmap(info, pixels, info.minRowBytes());
    SkMatrix mat = SkMatrix::Scale(1.0f / static_cast<float>(w), 1.0f / static_cast<float>(h));
    auto fontImage = SkImage::MakeFromRaster(pixmap, nullptr, nullptr);
    auto fontShader = fontImage->makeShader(SkSamplingOptions(SkFilterQuality::kLow_SkFilterQuality), mat);

    paint.setShader(fontShader);
    paint.setColor(SK_ColorWHITE);
    atlas.TexID = &paint;
}
}

namespace {
struct KeySymbolRemap
{
    KeySymbol   from;
    int         to;
    ImGuiKey_   maybeIdx;
};

KeySymbolRemap gKeySymbolRemap[] = {
        { KeySymbol::Key_Tab, 0, ImGuiKey_Tab },
        { KeySymbol::Key_Left, 1, ImGuiKey_LeftArrow },
        { KeySymbol::Key_Right, 2, ImGuiKey_RightArrow },
        { KeySymbol::Key_Up, 3, ImGuiKey_UpArrow },
        { KeySymbol::Key_Down, 4, ImGuiKey_DownArrow },
        { KeySymbol::Key_Page_Up, 5, ImGuiKey_PageUp },
        { KeySymbol::Key_Page_Down, 6, ImGuiKey_PageDown },
        { KeySymbol::Key_Home, 7, ImGuiKey_Home },
        { KeySymbol::Key_End, 8, ImGuiKey_End },
        { KeySymbol::Key_Insert, 9, ImGuiKey_Insert },
        { KeySymbol::Key_Delete, 10, ImGuiKey_Delete },
        { KeySymbol::Key_BackSpace, 11, ImGuiKey_Backspace },
        { KeySymbol::Key_space, 12, ImGuiKey_Space },
        { KeySymbol::Key_Return, 13, ImGuiKey_Enter },
        { KeySymbol::Key_Escape, 14, ImGuiKey_Escape },
        { KeySymbol::Key_A, 15, ImGuiKey_A },
        { KeySymbol::Key_a, 15, ImGuiKey_A },
        { KeySymbol::Key_C, 16, ImGuiKey_C },
        { KeySymbol::Key_c, 16, ImGuiKey_C },
        { KeySymbol::Key_V, 17, ImGuiKey_V },
        { KeySymbol::Key_v, 17, ImGuiKey_V },
        { KeySymbol::Key_X, 18, ImGuiKey_X },
        { KeySymbol::Key_x, 18, ImGuiKey_X },
        { KeySymbol::Key_Y, 19, ImGuiKey_Y },
        { KeySymbol::Key_y, 19, ImGuiKey_Y },
        { KeySymbol::Key_Z, 20, ImGuiKey_Z },
        { KeySymbol::Key_z, 20, ImGuiKey_Z }
};
}

ImGuiWindow::ImGuiWindow(Handle<DrawContext> drawCtx)
    : fDrawCtx(std::move(drawCtx)),
      fWindow(fDrawCtx->getWindow()),
      fContext(nullptr)
{
    fContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    for (const auto& remap : gKeySymbolRemap)
    {
        if (remap.maybeIdx >= 0)
            io.KeyMap[remap.maybeIdx] = remap.to;
    }

    io.Fonts->AddFontFromFileTTF("/home/sora/Project/C++/Cocoa/res/cantarell.ttf", 14.0f);
    build_im_font_atlas(*io.Fonts, fFontPaint);

    va_slot_connect(Window, Repaint, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onRepaint));
    va_slot_connect(Window, Close, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onClose));
    va_slot_connect(Window, ButtonPress, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onButtonPress));
    va_slot_connect(Window, ButtonRelease, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onButtonRelease));
    va_slot_connect(Window, Motion, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onMotion));
    // va_slot_connect(Window, KeyPress, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onKeyPress));
    // va_slot_connect(Window, KeyRelease, fWindow, sigc::mem_fun(*this, &ImGuiWindow::onKeyRelease));
}

ImGuiWindow::~ImGuiWindow()
{
    va_slot_disconnect(Window, Repaint);
    va_slot_disconnect(Window, Close);
    va_slot_disconnect(Window, ButtonPress);
    va_slot_disconnect(Window, ButtonRelease);
    va_slot_disconnect(Window, Motion);
    va_slot_disconnect(Window, KeyPress);
    va_slot_disconnect(Window, KeyRelease);

    ImGui::DestroyContext(fContext);
}

void ImGuiWindow::mouseMoveEvent(vec::float2 pos)
{
    ImGui::SetCurrentContext(fContext);
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = pos[0];
    io.MousePos.y = pos[1];
}

void ImGuiWindow::mouseButtonEvent(int idx, bool state)
{
    ImGui::SetCurrentContext(fContext);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[idx] = state;
}

void ImGuiWindow::mouseWheelEvent(float delta)
{
    ImGui::SetCurrentContext(fContext);
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel += delta;
}

void ImGuiWindow::onMotion(const Handle<Window>& win, vec::float2 pos)
{
    mouseMoveEvent(pos);
}

void ImGuiWindow::onButtonPress(const Handle<Window>& win, Button button, vec::float2 pos)
{
    mouseMoveEvent(pos);
    int idx = -1;
    switch (button)
    {
    case Button::kLeft:
        idx = 0;
        break;
    case Button::kRight:
        idx = 1;
        break;
    case Button::kMiddle:
        idx = 2;
        break;
    case Button::kWheelUp:
        mouseWheelEvent(1.0f);
        break;
    case Button::kWheelDown:
        mouseWheelEvent(-1.0f);
        break;
    case Button::kUnknown:
        return;
    }
    if (idx >= 0)
        mouseButtonEvent(idx, true);
}

void ImGuiWindow::onButtonRelease(const Handle<Window>& win, Button button, vec::float2 pos)
{
    mouseMoveEvent(pos);
    int idx = -1;
    switch (button)
    {
    case Button::kLeft:
        idx = 0;
        break;
    case Button::kRight:
        idx = 1;
        break;
    case Button::kMiddle:
        idx = 2;
        break;
    case Button::kWheelUp:
    case Button::kWheelDown:
    case Button::kUnknown:
        return;
    }
    if (idx >= 0)
        mouseButtonEvent(idx, false);
}

void ImGuiWindow::onRepaint(const Handle<Window>& win, const SkRect& rect)
{
    ImGui::SetCurrentContext(fContext);
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize.x = static_cast<float>(fWindow->width());
    io.DisplaySize.y = static_cast<float>(fWindow->height());

    // TODO: Initialize Alt, Ctrl, Shift, Super keys
    ImGui::NewFrame();

    signalPaint().emit();

    /* This causes ImGui to rebuild vertex/index data */
    ImGui::Render();

    /* Then we fetch the most recent data, and convert it so that we can render with Skia */
    const ImDrawData *drawData = ImGui::GetDrawData();
    SkTDArray<SkPoint> pos;
    SkTDArray<SkPoint> uv;
    SkTDArray<SkColor> color;

    SkCanvas *canvas = nullptr;

    for (int i = 0; i < drawData->CmdListsCount; i++)
    {
        const ImDrawList *drawList = drawData->CmdLists[i];

        pos.rewind();
        uv.rewind();
        color.rewind();
        for (int j = 0; j < drawList->VtxBuffer.size(); j++)
        {
            const ImDrawVert& vert = drawList->VtxBuffer[j];
            pos.push_back(SkPoint::Make(vert.pos.x, vert.pos.y));
            uv.push_back(SkPoint::Make(vert.uv.x, vert.uv.y));
            color.push_back(vert.col);
        }
        SkSwapRB(color.begin(), color.begin(), color.count());
        int indexOffset = 0;

        /* Draw everything with canvas.drawVertices */
        for (int j = 0; j < drawList->CmdBuffer.size(); ++j)
        {
            const ImDrawCmd* drawCmd = &drawList->CmdBuffer[j];

            SkAutoCanvasRestore acr(canvas, true);

            // TODO: Find min/max index for each draw, so we know how many vertices (sigh)
            if (drawCmd->UserCallback)
            {
                drawCmd->UserCallback(drawList, drawCmd);
            }
            else
            {
                auto paint = static_cast<SkPaint*>(drawCmd->TextureId);
                CHECK(paint);

                canvas->clipRect(SkRect::MakeLTRB(drawCmd->ClipRect.x, drawCmd->ClipRect.y,
                                                  drawCmd->ClipRect.z, drawCmd->ClipRect.w));
                auto vertices = SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                                                     drawList->VtxBuffer.size(),
                                                     pos.begin(), uv.begin(), color.begin(),
                                                     static_cast<int>(drawCmd->ElemCount),
                                                     drawList->IdxBuffer.begin() + indexOffset);
                canvas->drawVertices(vertices, SkBlendMode::kModulate, *paint);
                indexOffset += static_cast<int>(drawCmd->ElemCount);
            }
        }
    }
}

void ImGuiWindow::onClose(const Handle<Window>& win)
{
}

VANILLA_NS_END
