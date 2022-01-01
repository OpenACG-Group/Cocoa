# JCommentsSpec: JavaScript Comment Specification
**适用于 Cocoa 内部代码的 JavaScript 注释规范**

## Design Purpose & Principles
对于 Cocoa 来说，其内置的 JavaScript 代码（Internal Scripts）通常和原生 C++ 代码有很紧密的联系。
例如，`RefValue` 类在 `internal://context/refvalue` 中定义，有多处原生代码
（如 core 模块中的 Buffer 类）都直接使用到了它。由于通过 C++ 对 JavaScript 发起调用的逻辑相比起通过
JavaScript 直接调用要复杂得多，而二者又必须同时遵守某种调用约定（如函数的名称和参数顺序、数量等）
才能正确互相调用，因此在内部 JavaScript 中显式地注记出所遵守的调用约定，
以防止读者对代码的执行逻辑和存在的合理性产生困惑就显得尤为重要。例如，在 `RefValue` 类中
（实际上是在其构造函数原型上）有一个静态方法 `__has_instance__`，其功能等价于 `instanceof` 运算符，
用户一般不会用到它，但是在原生代码中它十分重要且会被频繁调用。不知晓这一点的代码读者，
在发现没有任何 JavaScript 代码对该静态方法有引用后，可能就会对这个有着与众不同命名方法的函数产生困惑。

为了解决上述问题，Cocoa 设计了 __JCommentsSpec__ 规范，它是一种 __注释规范__，不对代码本身产生影响。
JCommentsSpec 遵循以下设计原则：
1. 仅在注释上改动；
2. 保持高机器可读性和人类可读性；
3. 保持简洁；
4. 在不违反原则 3 的前提下，尽量传达更多的信息。


## Concepts
__空白符（Whitespace）__: 包含空格、水平制表符在内的不显示字符。

__注释前缀__: JavaScript 单行注释开头的双写正斜杠 `\\`

__调用约定（Calling Convention）__: 特指原生代码调用 JavaScript 时，使用的函数名、
填充的参数数量、类型和预期的返回值

## Specification
JCommentsSpec 一律仅适用于 __单行注释__（以 `\\` 开头），从行首到注释前缀，只能出现 0 个或多个空白符，
这是因为单行注释相比多行注释，更便于程序解析和查找，且明确的前缀便于快速字符串匹配。
JCommentsSpec 遵循基本格式：
```javascript
// #[[<directive>:<param#0,param#1...>]]
```
尖括号内的部分表示需要被替换为实际对应的内容。

`#[[...]]` 格式是整个 JCommentsSpec 的标志和框架，方括号内部的是实际的内容。

`<directive>` 称为 __制导部分__, 表示该条 JCommentsSpec 注释应该被如何解释，后文中有所有可用的
directives 以及对应的说明。

`<param#0,param#1...>` 称为 __参数列表__，参数是对 directive 的补充说明，有修饰和约束
directive 的作用。

### Directive `IScriptURL`
位于文件开头，表示该文件属于内部脚本（Internal Script），并指定该内部脚本的 URL。
- `param[0]` - [必须] 脚本的 URL（需要以 `internal://` 开头）
- `param[1]` - [可选] 脚本对应的 XML 文件名（仅包含文件名不包含路径，包含 xml 后缀名）

### Directive `IScriptScope`
位于文件开头，表示该文件属于内部脚本，并指定该内部脚本的作用域。
脚本的「作用域」指脚本的可用范围，例如一些内部脚本可以直接暴露给用户使用，另一些则只能 Cocoa 内部使用，
因为它们可能包含一些敏感代码。该 directive 接受 1-4 个参数，可以是以下四个字符串字面量的任意不重复排列组合：
- `UserImport` - 用户可作为模块导入
- `UserExecute` - 用户可以直接执行（或称为 Evaluate）
- `SysImport` - Cocoa 内部可作为模块导入
- `SysExecute` - Cocoa 内部可以直接执行（或称为 Evaluate）

### Directive `CCProtocol`
用于注释函数、类方法，表示该函数将被 Cocoa 内部调用（这通常意味着让用户忽略该函数并避免使用它），
并指明该函数遵循何种调用约定。接受一个参数，表示调用约定：
- `HasInstance` - 接受一个 `any` 参数 value，返回 `boolean`。若 value 是当前方法所属的类的实例，
返回 true，否则 false。具有固定函数名 `__has_instance__`，必须为类静态方法。