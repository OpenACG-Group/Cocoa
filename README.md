# Cocoa — Visual Novel Engine for Linux
This project belongs to [OpenACG Group](https://github.com/OpenACG-Group).

Primary maintainer is [Sora](https://github.com/wait1210day).

わたしわ、高性能ですから！

## Introduction
Windows and Android users have enjoyed a large number of visual novels while Linux users
have been suffering from a lack of VN engines. Cocoa is a project inspired by *ATRI -My Dear Moments-*,
aiming to improve the VN experience on Linux platform and provide a framework for those who are
interested in VN creating. 

It is TypeScript that is used for Cocoa's official programming language. Actually, Cocoa can be treated
as a JavaScript engine, which is written in C++20, with rendering functions. It consists of a series
"language bindings" to implement some functions natively. But a lot of necessary functions
like scenario parsing are implemented in TypeScript, which is based on native code.

Cocoa is still being developed and haven't been ready for commercial use.
Issues / Pull requests are welcome.


## Background & Pronunciation
As we all known, cocoa is a type of plant whose bean is the thing from which chocolate is made.
But _Cocoa_ here doesn't mean that. Instead it comes from an anime called _Is the Order a Rabbit?_
which has a heroine named _Kokoa Hoto_. So its pronunciation is actually follows the Japanese
Katakana 「ココア」, which is \kokoa\ in IPA. But it doesn't matter much if you pronounce it in
English way.

## Dependencies
Generally, Cocoa is based on Google's [V8 JavaScript Engine](https://v8.dev) and
[Skia Graphics Library](https://skia.org). V8 provides an execution environment
of JavaScript and Skia takes the responsibility for interacting with GPU or CPU rasterizer.

V8, Skia, and other dependencies like libsquash, ffmpeg, etc. are managed by
[third-party building script](script/build-third-party.py) which is written in Python. The
script can download (or clone for git repositories) and build dependencies automatically by following
instructions in [configuration file](third_party/package-lists.json). Most huge third-party libraries
will be built as shared objects (`.so` file), and other small dependencies like fmt and libuv will
be built as static libraries and linked to Cocoa directly.

## CLI Options


## TypeScript
Cocoa can only execute JavaScript directly. So TypeScript file must be compiled to JavaScript before
running them. You're supposed to use tools in [devtools](typescript/devtools) directory to compile
TypeScript files.

Native functions in Cocoa are not exported as global objects like the DOM model in web browser.
Instead, they are wrapped as ES modules which can be imported optionally:
```typescript
import * as core from 'core';
```
A *synthetic module* is **a module that only contains native exports**. Synthetic modules
have no file paths, which means you should import them just by thier name directly (like `'core'`).
Actually, synthetic modules are presented by a special format in Cocoa internally: name of
the module adds a prefix `synthetic://`. For instance, `core` module will be identified by
URL `synthetic://core`.

