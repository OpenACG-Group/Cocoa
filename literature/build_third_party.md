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

最后，回到 `third_party` 目录，准备下一个依赖的构建。

### 1.9. 构建 WebSockets 及网络支持库 libwebsockets
克隆 libwebsockets 的 Git 仓库：
```shell
$ git clone https://libwebsockets.org/repo/libwebsockets
```

进入目录，然后用 CMake 配置项目并编译：
```shell
$ cd libwebsockets
$ mkdir out
$ cd out
$ cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$BUILD_DIR \
        -DLWS_WITH_LIBUV=ON \
        -DLWS_WITH_STATIC=OFF \
        -DLWS_WITH_JSONRPC=OFF \
        -DLWS_WITHOUT_TESTAPPS=ON \
        -G Ninja \
        ..
$ ninja
$ ninja install
```

完成后，回到 `third_party` 目录，准备善后工作。

### 1.10. 善后工作
至此，所有的基本依赖已经编译完成，由于解包而产生的各种源文件目录已经可以删去，
只需保留 `build` 目录即可，`build` 目录中已经存放了所有的编译产物。

## 2. 构建 Google Skia 和 V8
~~（这位更是重量级）~~

下列命令大多数需要连接到 Google 服务器，这一点请用户自行解决。

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
$ gn gen out/release.shared --args='is_component_build=true skia_use_vulkan=true skia_use_gl=false is_official_build=true cc="clang" cxx="clang++" is_debug=false skia_use_system_icu=true'
$ ninja -C out/release.shared
```
编译耗时较长（ninja 已经自动开启并行编译），会占用较多的内存和 CPU 资源。

然后编译 V8：
```shell
$ cd ../v8
$ git checkout -b 9.9 -t branch-heads/9.9
$ mkdir out
$ gn gen out/release.shared --args="use_custom_libcxx=false is_debug=false is_component_build=true cc='clang' cxx='clang++'"
$ ninja -C out/release.shared
```
V8 编译耗时比 Skia 还要更长一些。

两个库都编译完成后，回到 `third_party` 目录，然后运行脚本，将编译产物复制到正确的位置：
```shell
$ cd $THIRDPARTY_DIR
$ ../script/deploy-v8-skia-artifacts.sh
```

V8 和 Skia 编译完成。

## 3. 构建 CanvasKit (WASM)
CanvasKit 是 Skia 项目的一部分，其编译产物是 WebAssembly 和 JavaScript，
而不是 ELF 二进制文件。

您可能是已经十分了解 WebAssembly 工具链的开发者，也可能完全没有使用过，
无论如何，本文尽可能详细地写出用 Emscripten 编译 CanvasKit 的步骤，
并假设您已经事先对 WebAssembly 有所了解。

### 3.1. 准备环境
为了编译 CanvasKit，需要事先准备 Emscripten SDK，Skia 已经准备了对应的下载脚本：
```shell
$ ./bin/activate-emsdk
```
若使用 HTTP 代理，可设置环境变量 `https_proxy` 指向对应的代理服务器地址。

执行完后，检查 `third_party/externals/emsdk` 目录下是否出现 `upstream` 目录，若出现则证明下载成功。

### 3.2. 配置环境并验证工具链
在 `third_party/externals/emsdk/upstream/emscripten` 下创建 `.emscripten` 文件，
写入如下配置，__复制前注意阅读注释__：
```python
# Note: If you put paths relative to the home directory, do not forget
# os.path.expanduser
#
# Any config setting <KEY> in this file can be overridden by setting the
# EM_<KEY> environment variable. For example, settings EM_LLVM_ROOT override
# the setting in this file.
#
# Note: On Windows, remember to escape backslashes! I.e. LLVM='c:\llvm\'
# is not valid, but LLVM='c:\\llvm\\' and LLVM='c:/llvm/'
# are.

# !!! 用真实的目录替换 /path/to/Cocoa
emsdk_base_dir = '/path/to/Cocoa/third_party/skia/third_party/externals/emsdk'

# This is used by external projects in order to find emscripten.  It is not used
# by emscripten itself.
EMSCRIPTEN_ROOT = emsdk_base_dir + '/upstream/emscripten' # directory
LLVM_ROOT = emsdk_base_dir + '/upstream/bin' # directory
BINARYEN_ROOT = emsdk_base_dir + '/upstream' # directory

# Location of the node binary to use for running the JS parts of the compiler.
# This engine must exist, or nothing can be compiled.
NODE_JS = '/usr/bin/node' # executable

JAVA = 'java' # executable
```

接下来，设定一个指向 emsdk 编译器目录的环境变量，以便我们接下来的测试：
```shell
$ export EMCC_BIN_DIR=${THIRDPARTY_DIR}/skia/third_party/externals/emsdk/upstream/emscripten
```

创建一个临时目录（例如 `/tmp/sketch`），进入该目录，编写一个 C 测试程序（Hello World），
这里提供一个参考示例：
```c
#include <stdio.h>
int main(int argc, const char **argv)
{
    printf("Hello, World!\n");
    return 0;
}
```
保存为 `test.c` 文件，然后使用如下命令尝试将该文件编译为 WebAssembly 程序：
```shell
$ ${EMCC_BIN_DIR}/emcc test.c
```
如果编译成功，应该会出现 `a.out.js` 和 `a.out.wasm` 两个文件，其中 JavaScript
文件保存有运行 WASM 需要的胶水代码。如果编译失败，请检查 Emscripten 工具链的配置是否正确。

用 Node 运行该 JavaScript，若得到 Hello World 输出，则证明 Emscripten 工具链配置成功：
```shell
$ node a.out.js
Hello, World!
```

离开临时目录，清理文件，进入 `${THIRDPARTY_DIR}/skia` 目录，正式开始构建 CanvasKit。

### 3.3. 构建 CanvasKit
配置并验证工具链可用后，使用下列命令构建 CanvasKit：
```shell
$ cd modules/canvaskit
$ ./compile.sh cpu no_canvas
```

最后，复制编译产物到正确的位置：
```shell
$ cd ${THIRDPARTY_DIR}
$ cp skia/out/canvaskit_wasm/canvaskit.js build
$ cp skia/out/canvaskit_wasm/canvaskit.wasm build
```

### 3.4. 注意事项
在编译完 skia 和 v8 后，不要移除它们的目录，因为 Cocoa 需要使用其中的头文件。

## 4. 后记 - 依赖如何被使用
对于编译出静态链接库的依赖，Cocoa 会将它们作为自身的一部分直接链接进可执行文件；
对于动态链接库，Cocoa 会以运行时动态链接的方式引入它们；
对于 CanvasKit 的 WebAssembly 和 JavaScript 文件，Cocoa 会将其打包进内置资源包，
并作为内置模块暴露给用户使用。

至此，所有的 Cocoa 依赖编译完成。
