Build Cocoa from Source Code - Dependencies
===========================================
从零开始的 Cocoa 构建


## Overview
构建 Cocoa，主要是需要构建 Cocoa 的众多第三方依赖库。这些库 Cocoa 本身已经提供了预编译的版本，
对于想要快速构建 Cocoa 的用户而言，建议使用预编译的库，只需在 OpenACG Group
的 `cocoa-prebuilt` 仓库中下载 `prebuilt.tar.gz`，将其直接解压到 `third_party` 目录下即可。

若不使用预编译的库，那么就需要 __手动构建 Cocoa 的所有依赖__，这会是一项比较困难的任务，
若不是预编译的库无法使用，或想要尝试探索 Cocoa 的依赖系统，我们不推荐进行手动构建。

本文的剩余部分将指导用户如何下载并构建依赖。下面介绍的步骤中有些相互关联，有些彼此独立，
但无论如何，请严格按照我们介绍的顺序进行构建。

## 1. 构建常规依赖
我们将首先从比较容易构建的依赖开始。

### 1.1. 准备环境
构建依赖需要按照若干软件包，请检查下列软件包是否已安装且在命令行中可用：
* Bash
* Clang Toolchain v5.0+ (C++17 support)
* GNU make
* CMake v3.0.0+
* Ninja 
* Python v3.10+
* GN building system
* Perl

所有的构建任务都在 `third_party` 目录下完成，所以先进入该目录：
```shell
$ cd third_party
```

在构建过程中，一些路径和参数会被反复用到，把它们设为环境变量会方便后序步骤的输入：
```shell
$ export COCOA_ROOT=<absolute path to Cocoa project>
$ export THIRDPARTY_DIR=${COCOA_ROOT}/third_party
$ export BUILD_DIR=${THIRDPARTY_DIR}/build

$ export CMAKE_OPTS=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang)
```

最后，创建一个 `build` 目录，该目录相当于一个虚拟的根目录，
此后所有的依赖库的头文件和二进制文件都将安装到此目录下，Cocoa 会直接从该目录寻找链接库；
再创建一个 `packages` 目录，用户下载的所有源码压缩包都应该存放在该目录下：
```shell
$ mkdir build packages
```

### 1.2. 构建适用于 C++ 的 JSON 解析库 `jsoncpp`
去往 [官方 GitHub 的 release 页面](https://github.com/open-source-parsers/jsoncpp/releases)
下载最新的源码到 `packages` 目录下，然后解包并进入目录（假设下载的包是 `jsoncpp-1.9.5.tar.gz`）：
```shell
$ tar xf packages/jsoncpp-1.9.5.tar.gz
$ cd jsoncpp-1.9.5
```

创建一个目录用于编译，然后配置项目：
```shell
$ mkdir out && cd out
$ cmake $CMAKE_OPTS -DCMAKE_INSTALL_PREFIX=$BUILD_DIR -G Ninja ..
```

编译并安装软件库到 `$BUILD_DIR`：
```shell
$ ninja && ninja install
```

这里我们使用了 `ninja` 来进行构建，也可以使用传统的 `make` 方式来构建，将 CMake 的参数
`-G Ninja` 去掉即可。

检查 `$BUILD_DIR` 下是否已出现文件 `lib/libjsoncpp.a` 和 `include/json/`，出现则证明构建成功。
返回 `third_party` 目录，准备下一个依赖的构建：
```shell
$ cd $THIRDPARTY_DIR
```

### 1.3. 构建内存分配器 `jemalloc`
从 [jemalloc 的官方 GitHub 页面](https://github.com/jemalloc/jemalloc/releases) 下载源码包，
经历和 jsoncpp 相似的解包、进入目录的步骤后（这里不再赘述），根据官方的 `INSTALL.md`，
我们直接在源码树内构建：
```shell
$ ./configure --prefix=$BUILD_DIR
$ make
$ make install
```

检查 `$BUILD_DIR` 下是否存在 `lib/libjemalloc_pic.a` 文件，存在则构建成功。
回到 `third_party` 目录，准备下一个依赖的构建。

### 1.4. 构建 C++ 格式化字符串库 `fmt`
从 [fmt 的官方 GitHub 页面](https://github.com/fmtlib/fmt/releases) 下载源码包，
经历相似的解包、进入目录步骤后，配置项目并构建：
```shell
$ mkdir out
$ cd out
$ cmake $CMAKE_OPTS -DCMAKE_INSTALL_PREFIX=$BUILD_DIR -G Ninja ..
$ ninja
$ ninja install
```

检查 `$BUILD_DIR` 下是否存在 `lib/libfmt.a` 和 `include/fmt/` 文件，存在则构建成功。
回到 `third_party` 目录，准备下一个依赖的构建。

### 1.5. 构建栈帧回溯支持库 `libunwind`
从 [libunwind 的官方 GitHub 页面](https://github.com/libunwind/libunwind/releases) 下载源码包，
经历相似的解包、进入目录步骤后，配置项目并构建：
```shell
$ CC=clang CXX=clang++ ./configure --prefix=$BUILD_DIR
$ make
$ make install
```

检查 `$BUILD_DIR` 下是否存在 `lib/libunwind.so` 和 `lib/libunwind.h` 文件，存在则构建成功。
回到 `third_party` 目录，准备下一个依赖的构建。

### 1.6. 构建异步事件循环库 `libuv`
从 [libuv 的官方 GitHub 页面](https://github.com/libuv/libuv/releases) 下载最新源码包，
经历相似的解包、进入目录步骤后，配置项目并构建：
```shell
$ mkdir out
$ cd out
$ cmake $CMAKE_OPTS -DCMAKE_INSTALL_PREFIX=$BUILD_DIR -G Ninja ..
$ ninja
$ ninja install
```

检查 `$BUILD_DIR` 下是否存在 `lib/libuv_a.a` 以及 `include/uv` 文件，存在则构建成功。
回到 `third_party` 目录，准备下一个依赖的构建。

### 1.7. 构建虚拟文件系统 crpkg 支持库 `libsquash`
从 [libsquash 的官方 GitHub 页面](https://github.com/pmq20/libsquash/releases) 下载最新源码包，
经历相似的解包、进入目录步骤后，配置项目并构建：
```shell
$ mkdir out
$ cd out
$ cmake $CMAKE_OPTS -DCMAKE_INSTALL_PREFIX=$BUILD_DIR -G Ninja ..
$ ninja
```

此时需要注意，`libsquash` 不能通过 `ninja install` 来直接安装到虚拟目录，我们需要手动复制文件：
```shell
$ cp libsquash.a $BUILD_DIR/lib
$ cp -r ../include/* $BUILD_DIR/include
```

回到 `third_party` 目录，准备下一个依赖的构建。

### 1.8. 构建多媒体编解码库 FFmpeg
从 [FFmpeg 官网](http://www.ffmpeg.org/releases) 下载最新的源码包，
经历相似的解包、进入目录步骤后，配置项目并构建：
```shell
$ mkdir out
$ cd out
$ ../configure --prefix=$BUILD_DIR \
               --cc=clang \
               --dep-cc=clang \
               --cxx=clang++ \
               --enable-gpl \
               --enable-version3 \
               --disable-everything \
               --disable-all \
               --disable-static \
               --enable-avcodec \
               --enable-avformat \
               --enable-avutil \
               --enable-swresample \
               --enable-fft \
               --enable-rdft \
               --enable-libopus \
               --enable-shared \
               --enable-hwaccels \
               --enable-vaapi \
               --disable-bzlib \
               --disable-iconv \
               --disable-network \
               --disable-schannel \
               --disable-sdl2 \
               --disable-symver \
               --disable-xlib \
               --disable-zlib \
               --disable-securetransport \
               --disable-faan \
               --disable-alsa \
               --disable-autodetect \
               --enable-decoder=vorbis,libopus,flac \
               --enable-decoder=pcm_u8,pcm_s16le,pcm_s24le,pcm_s32le,pcm_f32le,mp3 \
               --enable-decoder=pcm_s16be,pcm_s24be,pcm_mulaw,pcm_alaw \
               --enable-demuxer=ogg,matroska,wav,flac,mp3,mov,flv \
               --enable-parser=opus,vorbis,flac,mpegaudio,vp9,webp,mjpeg,gif,png \
               --enable-decoder=acc,h264 \
               --enable-demuxer=aac \
               --enable-parser=aac,h264 \
               --disable-linux-perf \
               --enable-lto \
               --enable-pic
$ make
$ make install
```

配置十分冗长，我们对 FFmpeg 做了一些裁剪工作，防止编译产物过于臃肿。该配置仅仅编译了 FFmpeg
的解复用、解码、硬件加速解码部分。`configure` 执行时不会产生详尽的日志输出，并且会耗费约 15s 左右的时间，
耐心等待即可。FFmpeg 代码量较多，建议在 `make` 时可以添加 `-jN` 参数开启并行编译，`N` 可以是 CPU 核心数。

完成后，回到 `third_party` 目录，准备善后工作。

### 1.9. 善后工作
至此，所有的基本依赖已经编译完成，由于解包而产生的各种源文件目录已经可以删去，
只需保留 `build` 目录即可，`build` 目录中已经存放了所有的编译产物。

## 2. 构建 Google Skia 和 V8
~~（这位更是重量级）~~

2D 绘图库 Skia 和 JavaScript 引擎 V8 是 Cocoa 最重要、最庞杂、也是最难以构建的两个依赖。

下列命令大多数需要连接到外网，这一点请用户自行解决。

首先，参照 [Google 官方的指示](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)，
部署 depot_tools：
```shell
$ git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
$ export PATH=${PATH}:${THIRDPARTY_DIR}/depot_tools
```

更新 depot_tools：
```shell
$ gclient
```

获取 V8, Skia 源码和依赖：
```shell
$ fetch v8
$ fetch skia
```
两个项目包含依赖的完整源码十分庞大，达到了上 GB 级别，请耐心等待下载完成。

下载完成后，进入 `skia` 目录，开始编译 Skia：
```shell
$ cd skia
$ mkdir out
$ gn gen out/Shared --args='is_component_build=true skia_use_vulkan=true skia_use_gl=false is_official_build=true cc="clang" cxx="clang++" is_debug=false skia_use_system_icu=true'
$ ninja -C out/Shared
```
编译耗时较长（ninja 已经自动开启并行编译），会占用较多的内存和 CPU 资源。

然后编译 V8：
```shell
$ cd ../v8
$ git checkout -b 9.9 -t branch-heads/9.9
$ mkdir out
$ gn gen out/shared --args="use_custom_libcxx=false is_debug=false is_component_build=true cc='clang' cxx='clang++'"
$ ninja -C out/shared
```
V8 编译耗时比 Skia 还要更长一些。

两个库都编译完成后，回到 `third_party` 目录，然后运行脚本，将编译产物复制到正确的位置：
```shell
$ cd $THIRDPARTY_DIR
$ ../script/deploy-v8-skia-artifacts.sh
```

V8 和 Skia 编译完成。

至此，所有的 Cocoa 依赖编译完成。
