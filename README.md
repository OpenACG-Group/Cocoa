# Cocoa — Visual Novel Engine for Linux
This project belongs to [OpenACG Group](https://github.com/OpenACG-Group).

Primary maintainer is [Sora](https://github.com/wait1210day).

わたしわ、高性能ですから！

## Introduction
长久以来，Linux 平台都缺乏稳健可靠的视觉小说框架，而 Windows 和 Android 平台则都有对应版本的
[krkr 解释器](https://github.com/krkrz/krkrz)。Cocoa 基于 C++20 为 Linux 平台实现了一个高性能、
现代化的视觉小说引擎。该项目仍然处于开发实验阶段，大量功能正在实现中，欢迎提交 Issue / Pull Requests。

## Dependencies
### Building tools
- `Clang` 建议使用的 C/C++ 编译器
- `CMake` 配置和构建项目
- `Python (>=3.0)` 运行脚本，生成编译时必须的文件

### Runtime dependencies
- `Skia` 2D 绘图基本库
- `V8` JavaScript 引擎
- `libyuv` 用于视频解码器的 YUV 颜色格式到 RGB 颜色格式的转换
- `libuv` 基于事件的异步编程库，为 JavaScript 异步功能提供基本支持，实现事件主循环
- `libav` ffmpeg 的一系列库（avformat, avcodec, swscale, swresample 等），用于音视频解码
- `Vulkan` 基于 GPU 加速的 2D 绘图支持
- `XCB` X11 协议，基本窗口系统的实现
- `xkbcommon` 键盘状态的维护、不同键盘布局的统一抽象
- `pulseaudio` 音频输出服务
- `fmt` 日志格式化
- `libsquash` 在用户空间实现的 squashfs 文件系统库，为 crpkg 归档文件提供了支持

## Building System
Cocoa 的构建系统主要由 CMake 负责。为了管理复杂的外部依赖库，并降低 CMake 规则的复杂度，
OpenACG Group 同时还规定了一些额外的编译规则：

### package-lists.json
在 [`third_party`](./third_party) 目录下有 [`package-lists.json`](./third_party/package-lists.json)
文件，该文件是脚本 [`build-third-party.py`](./script/build-third-party.py) 的配置文件。该 JSON
文件指导脚本如何去下载并构建所需的依赖项。关于该文件的详细说明，参见 `docs` 下的文档。

