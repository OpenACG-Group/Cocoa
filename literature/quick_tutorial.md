Quick Tutorial
==============

这篇快速指南将指导用户如何基于 TypeScript 编写 Cocoa 程序，并渲染实际内容。

这篇教程不是一个 Cocoa 使用的完全指南，在这里我们仅仅用 CanvasKit 来渲染内容，
有关 [图层树和结构化渲染](./pipeline/rasterization_stage.md) 等内容，请参阅 API 文档。

## Hello World in TypeScript
Cocoa 的 `core` 内建模块提供了访问系统 IO 和操作内存的基本功能。
