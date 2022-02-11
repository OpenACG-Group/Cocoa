# literature/essay: Diary of Developers

## 11 Feb 2022, Fri

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
