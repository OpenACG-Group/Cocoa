# ECMAScript6 Module - URL Resolution & Synthetic Modules

## 1. Introduction
自 ES6 标准起，JavaScript 引入原生的 module 语法，Cocoa 对此也有相应的支持，
并且 ES6 模块是 Cocoa 唯一支持的模块语法，其它例如 CommonJS 或 AMD 的解决方案，Cocoa 一概不支持。

本文将逐步介绍 ES6 在 Cocoa 中的用法，以及一些 ES6 中没有提到的、Cocoa 扩充的概念。

## 2. URL Resolution
JavaScript 可以使用如下语法引入模块：
```javascript
import {Object} from 'URL';
```

其中 `URL` 部分指定了一个模块标识符（V8 称为 Specifier），告诉我们去哪里获取到模块代码。
Cocoa 规定，__标准的模块 Specifier__ 应当遵循如下格式：
```
<scheme>://<path>
```
`scheme` 部分表明后面的 `path` 是何种类型的路径，可能的 `scheme` 有：
+ `file` 指定一个存在于文件系统上的模块；
+ `internal` 指定一个 Cocoa 的内建模块；
+ `synthetic` 指定一个 Synthetic Module，它只包含原生代码。

### 2.1. `file` scheme
`file` scheme 很简单，它要求 `path` 是一个存在于文件系统上的路径。
如果使用相对路径，则该路径是以该源文件所在的目录作为当前目录。

例如，在 `/some/path/loader.js` 中有如下 import 语句：
```javascript
// 注意这里 file:// 之后直接是 other.js
import * as Some from 'file://other.js';
// 或者：
import * as Some from 'file://./other.js';
```
则被引入的 JavaScript 模块是 `/some/path/other.js`。

若使用绝对路径，则可以直接 `file:///some/path/other.js` 即可。

### 2.2. `internal` scheme
`internal` 用于引入 Cocoa 内建的 JavaScript 模块，例如：
```javascript
// 加载内建模块 /some/path/some.js
import * as Some from `internal:///some/path/some.js`;
```

需要注意的是，虽然内建模块也是以类似 UNIX 路径的方式表示的，但是它没有类似 `file` scheme
那样的相对路径解析规则，原则上来说内建模块应当只允许绝对路径，但是为了方便书写 Cocoa 也允许省略第一个 `/`。
即，`internal://some.js` 和 `internal:///some.js` 是等价的。

### 2.3. `synthetic` scheme
Synthetic Module 类似于内建模块，但由 `internal` 标识的内建模块是由 JavaScript 书写的，
但是 Synthetic Module 则是 __完全不包含 JavaScript 代码的原生模块__。
每一个 Synthetic Module 的导出对象都对应一个 C++ 函数、类或对象。

更进一步，Synthetic Module 不一定就是 Cocoa 内建的，
通过 `--runtime-preload` 命令行参数或者 `introspect.loadSharedObject` 函数，可以从外部加载一个共享库文件，
这些共享库也可能会提供 Synthetic Module。

毫无疑问，Synthetic Module 就是 Cocoa 实现 __语言绑定（Language Binding）__ 的机制，
关于语言绑定的更多信息，参见 [此处](./language_bindings.md)。

`synthetic` scheme 可以指定加载一个 Synthetic Module，`path` 部分此时不再是路径，
可以是任意字符串，表示该 Synthetic Module 的名称。

例如，加载 `core` 模块：
```javascript
import * as std from 'synthetic://core';
```

### 2.4. Canonical Specifier
标准化的 specifier 用于 Cocoa 在内部标识一个 JavaScript 模块，定义如下：
+ 对于 `file` scheme，标准化 specifier 中 `path` 是绝对路径，如果有符号链接，
则递归地解析符号链接，直到得到真正的文件路径；
+ 对于 `internal` scheme，标准化 specifier 中 `path` 一定是绝对路径，以 `/` 开头；
+ 对于 `synthetic` scheme，由于没有使用路径，以 `synthetic://specifier` 格式给出的 URL 即为标准化 specifier。

使用标准化 specifier 来作为内部的模块标识符的唯一理由是，它保证了对于某种特定 scheme 的唯一性，即，
绝对路径在系统中是唯一的，在内建模块中也是唯一的，这为 Cocoa 建立 Module Cache 提供了基础。

### 2.5. Fallback
方便起见，Cocoa 允许省略 URL 中的一些部分，以缩短 URL 的书写，这种情况下，
Cocoa 将自动尝试补全 URL，并按照一般 URL 进行解析。

如果 scheme 部分被省略，则 Cocoa 会先将其当作 `synthetic` 处理，
若 `path` 部分不能匹配到一个合适的 Synthetic Module，则再将其作为 `file` 处理，
若 `path` 也不是一个有效的文件系统路径，则判定为无效 URL。可见，`internal`
不在自动补全之列，它总是需要显式的写出。

对于 `file` 和 `internal`，JavaScript 文件的拓展名也可以省略，
这种情况下，Cocoa 会按顺序尝试 `.js` 和 `.mjs` 拓展名，如果都没有成功，则判定为无效 URL。

示例：
```typescript
import * as std from 'core';
// => synthetic://core

import * as CanvasKit from 'internal://canvaskit';
// => internal:///canvaskit.js

// Source file is /path/to/some/foo.js
import {Some} from './bar';
// => file:///path/to/some/bar.js
```

## 3. Dynamic Import
__动态导入（Dynamic Import）__ 和 __静态导入（Static Import）__ 遵循完全一致的 URL 解析方式。

## 4. Module Cache

### 4.1. [V8 Specified] Compilation & Instantiation of Module
当运行一个 JavaScript 脚本时，__编译（Compile）__ 和 __求值（Evaluate）__ 是必经的两个步骤，
后者真正地运行 JavaScript 代码并得到执行结果。

### 4.2. Module Cache
考虑下面一组 JavaScript 模块：
```typescript
// common.js:
import * std from 'core';

// foo.js:
import {Some} from './common.js';

// bar.js:
import {Some} from './common.js';
import {OtherSome} from './foo.js';
```

运行 `bar.js`，