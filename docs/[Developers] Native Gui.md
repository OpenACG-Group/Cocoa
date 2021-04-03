# Native Graphic User Interface (NaGui)
<span style="font-family: Georgia, serif; font-style: italic; font-size: medium">
Developer's Manual Chapter III
</span>

---------------
## Introduction
It's necessary for Cocoa to provide a graphic user interface for internal tools such as
debugger and dialogs. Although we can complete everything by CLI, we'd better to provide
a prettier and easier interface for users.

__NaGui__ is an immediate mode GUI implementation which is based on Ciallo 2D engine.
Immediate Mode GUI (also known as IMGUI) is a GUI design pattern which doesn't have global
state. All the widgets in it is stateless and will be totally repainted in each frame.
Actually, __NaGui__ is based on [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear),
a tiny but powerful IMGUI library written in ANSI C.

There's also another pretty IMGUI library called [Dear ImGui](https://github.com/ocornut/imgui),
but we don't use it as the backend because it outputs vertex buffer which is designed for GPU
rendering instead of drawing operations.

See also: [Immediate Mode GUI in Wikipedia](https://en.wikipedia.org/wiki/Immediate_mode_GUI)

All the source files of NaGui is in `src/NaGui`.

## Context and Windows
`NaGui::Context` is a [UniquePersistent](%5BDevelopers%5D%20Core%20Module.md#UniquePersistent)
object. It should be initialized by `NaGui::Context::New()` before you create any windows,
and should keep alive util the last window is closed. To release it, call
`NaGui::Context::Delete()`.

`NaGui::Window` and `NaGui::WindowListener` is NaGui's abstraction of a window in window
system (Or a `Drawable` object in Ciallo).

### Lifecycle of a window
TODO: Complete this.
