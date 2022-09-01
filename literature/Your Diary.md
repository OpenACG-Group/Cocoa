# Your Diary: Diary of Developers
开发者日志. 同时也致敬我个人非常喜欢的视觉小说「Your Diary」.

## 11 Feb 2022, Fri.

* ### Cocoa 开发者日记开始记录。
本文件是 Cocoa 的开发随笔，会按日期分类，记录一些开发中的零碎细节。
本文件会被定期整理为 docs 下的正式文档。

* ### 切换到 V8 9.9 版本，支持 toplevel await
Cocoa Koi 模块切换到了 V8 v9.9，支持了顶层 await 和动态 `import()`。
动态 `import()` 功能尚且在完善中。
目前 9.x 版本的 V8 存在编译失败的问题，具体问题和解决过程详见 [Elegia](./elegia.md) 中本日的记录.
另外，POSIX 文件系统 API 的支持工作正在进行中。

* ### 2D 渲染后端和窗口系统架构改变，重心移向 Wayland 后端
原先的 `Vanilla` 渲染后端被完全废弃，期间尝试基于 GTK+ 设计了 `Gsk` 后端，但是最终发现代码可读性不高，
架构不清晰，要实现的功能太多太杂，再次废弃。现在全新的 `Cobalt` 后端正在开发中。
`Cobalt` 运行在独立的 `RenderThread` 线程中，
这使渲染部分的事件循环（主要和窗口系统交互）同 V8 主线程的事件循环分离开来，
二者使用 libuv 提供的 `uv_async_t` handle 实现非阻塞线程间通讯。

详细来说：主线程称为 RenderHost 方，渲染线程称为 RenderClient 方。
RenderThread 中所有需要对外暴露接口的对象全部被抽象为对应的 `RenderClientObject` 对象，
主线程只能持有 `RenderClientObject` 对象实例，不能获得实际的对象。
例如，渲染线程内部创建了一个 `Display` 对象，标识和窗口系统的一个连接实例，
而主线程不能拿到指向该对象的指针，只能拿到一个 `RenderClientObject` 对象，
该对象就「代表」了实际身处渲染线程的 `Display` 对象。远程对象上的每一个方法都被表示为唯一的 opcode。
要调用远程对象的方法时，例如 `Display` 对象上具有方法 `close`，
我们将其表示为操作码 `COBALT_OP_DISPLAY_CLOSE`。将该操作码和调用参数传入
`RenderClientObject::Invoke` 方法，该方法会立即返回，而在之后的某个时间点，
操作码代表的方法会尽快在渲染线程上被调用。主线程还可以在 `Invoke` 方法中传入一个回调函数，
当方法调用完成时，回调会被调用（在主线程的栈上）。
类似地，RenderClient 还可以主动地向渲染线程发送信号（Signal），
主线程可以主动地注册名为槽（Slot）的回调函数，当渲染线程发送（Emit）某个信号时，
所有的槽函数会被调用（在主线程的栈上）。

后端重心迁移：鉴于 X11 架构过于古老，API 过于诡异，暂时停止对 X11 的支持。
将开发重心转移至 Wayland。

## 20 Feb 2022, Sat.
* ### 新的 `Comiket` 子项目
启动了名为 `Comiket` 的 Python 子项目，目的是打造一个十分简易的视觉小说剧本解析和展示程序。
`Comiket` 基于 curses 库，在 Linux 标准控制台运行，不包含任何图形支持，只有最基本的剧本解析和文字排版功能。
一方面，`Comiket` 是一次对设计视觉小说剧本语法的尝试，另一方面，它在未来可能会被用于编写交互式的 Cocoa
使用指南。

* ### 完善语言绑定
`core` 中的大部分文件系统 API 已经完成（除目录有关的 API 外）。
新的 `cobalt` 语言绑定的基本框架部分已完成。

## 5 Mar 2022, Sat.
这两周里断断续续做了许多零碎的工作
* ### 使用 jemalloc 内存分配器
为了提高内存分配效率，以及方便进行 heap profiling，Cocoa 切换到了 jemalloc 内存分配器。
jemalloc 在编译时直接静态链接进 Cocoa 可执行文件，默认开启 heap profiling 功能。
可以在 JavaScript 中实时生成当前堆状态的 dump：
```typescript
import * as std from 'core';

// Generate a dump file of heap statistics.
// File will be stored in current directory.
std.dumpNativeHeapProfile();
```

* ### 重构 `cobalt` 语言绑定基本框架
我们把有关 `Cobalt` 的信号槽机制和异步调用机制的胶水代码抽离出了 `Exports.h` 文件，
单独放在 `PromiseHelper.h` 和 `PromiseHelper.cc` 下。
并把这部分代码的主要工作抽象成两个闭包辅助结构体 `PromiseClosure` 和 `SlotClosure`。
现在，要编写实现异步调用的胶水代码的话：
```cpp
#include "Gallium/bindings/cobalt/PromiseHelper.h"
// ...

GALLIUM_BINDINGS_COBALT_NS_BEGIN

// A function that exported to JavaScript
v8::Local<v8::Value> method()
{
    // some operations...

    // converter_callback is a function (usually a lambda expression)
    // to convert a RenderHostCallbackInfo to the corresponding JS value to resolve the promise.
    // converter_callback can be nullptr, which means the promise will be resolved as 'undefined'.
    auto closure = PromiseClosure::New(isolate, converter_callback);

    // obj is a RenderClientObject
    obj->Invoke(opcode, closure, PromiseClosure::HostCallback /* other arguments... */);

    return closure->getPromise();
}

GALLIUM_BINDINGS_COBALT_NS_END
```

## 4 Apr 2022, Sun.
* ### Cobalt Pipeline 设计初步完成
我们为 Cobalt 设计了新的 2D 渲染流水线。

`RenderClient` 和 `RenderHost` 对象由 `GlobalContext` 对象维护，
该对象的类继承自 `UniquePersistent<>`，是一个典型的单例模式的对象，一个 Cocoa 进程中至多存在一个 `GlobalContext`。
JavaScript 通过语言绑定，向 `RenderClientObject` 对象发送调用或监听其信号。

一切由 `Display::Connect(...)` 开始，该函数返回一个 `Display` 对象，表示一个到系统的 Display 的连接，
例如 Wayland。用 `Display::CreateRasterSurface` 或 `Display::CreateHWComposeSurface` 创建可用于绘制的 `Surface` 对象，
该对象直接对应了一个独立窗口，`Surface` 内部包含了 `RenderTarget` 对象，`RenderTarget` 对象负责所有与渲染相关的功能，
而 `Surface` 本身负责事件处理和常规的窗口操作等与渲染无关的部分。`CreateRasterSurface` 和 `CreateHWComposeSurface`
创建的窗口具有不同类型的 `RenderTarget`，前者使用 CPU 软件光栅化，后者使用基于 Vulkan 的硬件光栅化。

`HWComposeContext` 是硬件光栅化的上下文，通常代表了一个 Vulkan 实例，所有 `Surface` 共享同一个 `HWComposeContext`，
但不同的 `Surface` 对象（即不同的 `RenderTarget` 对象）会创建属于自己的 `HWComposeSwapchain` 对象。
`HWComposeSwapchain` 实现了 GPU 缓冲区的管理，以及与窗口系统交互等功能（Vulkan WSI），它是 Vulkan 交换链的抽象。

`Blender` 对象是一个混成器，光栅化逻辑部分在此对象中完成。`Blender` 可以指定一个 `Surface` 用于输出合成的内容，
它将 layer tree 中的图层（`Layer` 对象）按照一定的顺序进行光栅化和混成，然后将结果输出到目标 `Surface` 上。
`Blender` 的光栅化和混成能力是由 `RenderTarget` 提供的，而 `Blender` 本身可以接受多种图层，其中最常见的是 `PictureLayer`，
该图层产生一个 `SkPicture` 对象，包含了一系列 2D 绘图指令。

图层是 Cobalt 能提供的最高层次抽象，`SkPicture` 的生成已经不是 Cobalt 的职责了，这由用户 JavaScript 来完成。

## 29 May 2022, Sun.
用业余时间断断续续地做了许多工作，渲染管线有一些调整。

* ### Cobalt 更名为 Glamor，Koi 更名为 Gallium
原先的 2D 渲染引擎更名为 `Glamor`，JavaScript 引擎更名为 `Gallium`。此次重命名早在上个月就已经完成，
今天才记录下来。

* ### TypeScript 渲染基础设施（TRI）：CanvasKit
CanvasKit 是一系列用 TypeScript 编写的类似 Web 中的 canvas 的一系列 2D 渲染接口。
CanvasKit 的职责是和 Glamor 模块交互，生成 `SkPicture` 对象，
然后由 `Compositor` 类（见 [compositor.ts](../typescript/canvaskit/compositor.ts)）将这些 `SkPicture`
对象混成、叠加、变换，输出到目标窗口中（由 Glamor 提供的 `Blender` 完成）。

CanvasKit 本身如何与 C++ 层交互尚且没有决定，目前我们尝试了基于二进制字节码的中间表示形式，
即由 CanvasKit 生成类似于汇编代码的用于表示绘图指令的 IR 字节码，但是很快我们就发现这种解决方案十分占用内存，
且我们不确定「编码 - 解码 - 解释执行」这一流水线会对绘图性能产生多大影响，
因此我们还同时在开发一种基于 [asmjit](https://github.com/asmjit/asmjit) 的解决方案。
在这种方案下，基于 JIT 技术进行代码生成，编译出类似 OpenGL 着色器的程序代码，然后由它生成 `SkPicture`.

用于和 CanvasKit 交互的 C++ 代码在 Glamor 的 [Moe 子模块下](../src/Glamor/Moe/MoeJITContext.h).

* ### Blender 类与 tiled rendering
