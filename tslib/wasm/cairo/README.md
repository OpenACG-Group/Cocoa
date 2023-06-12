# Integrate Cairo library by WebAssembly

## Introduction
Cairo is the first library that we attempted to compile into WebAssembly
and load by Cocoa. We have tried many ways to integrate some native C/C++
libraries into Cocoa by compling them into WASM directly instead of writing
a language binding of Cocoa, and finally we chose Cairo as our first try,
which is a popular graphics library like Skia.

Thanks to the [pango-cairo-wasm](https://github.com/VitoVan/pango-cairo-wasm)
project, it allows us to build Cairo and its dependencies into WASM easily.

## Build Cairo
Before actually building this integration project, you should build Cairo
and its dependencies into WASM. This can be done with the help of
[pango-cairo-wasm](https://github.com/VitoVan/pango-cairo-wasm) project:

```shell
# In the root directory of Cocoa project
$ cd third_party
$ git clone https://github.com/VitoVan/pango-cairo-wasm
$ cd pango-cairo-wasm
$ git submodule init && git submodule update
```

Then open the `env.sh` file, you will see something like:
```bash
################
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://www.wtfpl.net/ for more details.
################

export magicprefix=${magicdir}/build
## ...
```

Add a line and change line, then save the file:
```bash
## ...

# Remember to replace `<Cocoa project>` with the absolute path
# of Cocoa project
export magicdir=<Cocoa project>/third_party/pango-cairo-wasm
export magicprefix=<Cocoa project>/third_party/wasm-build

## ...
```

If you are not using Fedora, open another file `build.sh`:
```bash
################
# This program is free software. It comes without any warranty, to
# the extent permitted by applicable law. You can redistribute it
# and/or modify it under the terms of the Do What The Fuck You Want
# To Public License, Version 2, as published by Sam Hocevar. See
# http://www.wtfpl.net/ for more details.
################

sudo dnf -y groupinstall "Development Tools"
sudo dnf -y install python \
     ragel \
     byacc \
     flex \
     autoconf \
     automake \
     lbzip2 \
     gettext \
     autogen \
     libtool \
     gperf \
     gettext-devel \
     meson \
     ninja-build \
     gcc-c++ \
     libattr \
     pkg-config \
     which \
     cmake \
     glib2-devel

set -x
## ...
```

Comment the first two `dnf` commands, which is the package manager command of
Fedora, and you should manually check whether those required packages have been
installed on your system. But if you are a user of Fedora, you don't need to do
this.

Now the preparation steps have been done, and we can start to compile:
```shell
$ mkdir ../wasm-build
$ bash ./build.sh
```

It will take 20 minutes or more.

When building is finished, check the `//third_party/wasm-build` directory,
there should be some directories like Unix root filesystem:

```shell
$ ls ../wasm-build
bin  etc  include  lib  share  var
```

## Build Cairo binding
When the `//third_party/wasm-build` directory is prepared, CMake can be used
to build this integration project:

```shell
# In the root directory of Cocoa project
$ mkdir -p out/cairo.wasm
$ cd out/cairo.wasm

# This environment variable is not necessary.
# It is just used to shorten the CMake command.
$ EMSCRIPTEN_DIR=<Cocoa project>/third_party/pango-cairo-wasm/emsdk/upstream/emscripten
$ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake \
        -G Ninja \
        ../../tslib/wasm/cairo

$ ninja
```

The building artifact is a `CairoBinding.wasm` file and a `CairoBinding.js` file.

## Use Cairo
We have a TypeScript example at [`//tslib/samples/wasm-cairo-hello.ts`](../../samples/wasm-cairo-hello.ts)
that shows how to render a bitmap in memory with Cairo.
And in `//tslib/samples` directory, files whose name starts with `wasm-cairo`
can show you many examples of how to use Cairo API.

There is also a special example
[`//tslib/samples/wasm-cairo-sharing-composite.ts`](../../samples/wasm-cairo-sharing-composite.ts),
which shows how to let SkNative and Cairo share the same bitmap
render target. It can save copying of data between Cairo and JS,
but you must manage memory properly and carefully to avoid leaking.
