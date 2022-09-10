Cocoa Documentation for Users and Developers
============================================

Cocoa 项目文档索引

## 日志与杂文（不含文档内容）
* [Cocoa 第三方库缺陷与修复日志](./elegia.md)
* [随笔：ACG，爱，与童年](./komorebi/ACG%20Love%20and%20Childhood.md)

## 构建
* [从零开始的 Cocoa 依赖构建](./build_third_party.md)

## 基本组件
* 基本框架概览
* 错误处理，栈帧回溯，与信号量捕获
* [日志系统 Journal](./infrastructure/journal.md)
* 基本数据类型
* 属性树（Property Tree）
* CrPKG 资源归档与虚拟文件系统
* 事件循环、异步 IO 与线程池

## JavaScript 运行时
* JavaScript 运行时概览
* [ECMAScript6 模块](./runtime/es6_module.md)
* JavaScript 执行引擎与事件循环
* Infrastructure 与 Introspect
* [JavaScript 语言绑定](./runtime/language_bindings.md)

## 渲染机制与底层实现
* [2D 渲染流水线：概览](./pipeline/overview.md)
* [GLAMOR 渲染线程：独立资源管理、跨线程信号与跨线程方法调用](./pipeline/rendering_thread.md)
* ~~Deprecated: [CanvasKit 命令式绘图 API（TypeScript 与 C++ 层）](./CanvasKit.md)~~
* 从 Skia 导出的核心数据类型
* 2D 渲染流水线：导入 - 从核心绘图 API 到结构化渲染
* 2D 渲染流水线：JavaScript Paint 阶段
* [2D 渲染流水线：场景、图层树与光栅化阶段](./pipeline/rasterization_stage.md)
* [2D 渲染流水线：平台相关抽象（窗口系统集成）](./pipeline/window_system_integration.md)

## 构建系统与工具脚本
* 用 `modulec.py` 来创建自定义语言绑定
* 依赖部署

