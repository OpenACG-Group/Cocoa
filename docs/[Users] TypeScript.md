# Cocoa Script Language Specification
Cocoa 引擎是基于脚本语言的，__TypeScript__ 是 Cocoa 的官方脚本语言。__Koi__
是 Cocoa 中基于 V8 的 JavaScript 引擎。

## Overview
Koi 模块的全部源代码位于 `src/Koi` 目录下，所有相关类、函数等位于 `cocoa::koi`
命名空间下，在下文的介绍中我们默认省略命名空间前缀。

`Runtime` 对象对应了 V8 中的一个虚拟机实例，即 `v8::Isolate`，一个 Cocoa
进程只有唯一一个 `Runtime` 对象实例。此外，`Runtime` 对象同时维护了 JavaScript
运行时环境所需的一些其它资源，如 V8 上下文 `v8::Context`，ES6 模块缓存等。
以下代码演示了使用 `Runtime` 对象的一般过程：
```cpp
using namespace cocoa;
// 在此之前初始化 Journal, EventLoop 对象

// 加载内置的核心语言绑定（原生代码调用）
koi::PreloadInternalBindings();
// 可选地从一个动态库加载额外语言绑定，关于语言绑定的细节和规范，
// 参见下一节「Language Binding」
koi::PreloadBindingsFromDynamicLibrary(...);

auto rt = koi::Runtime::MakeFromSnapshot(...);
// Root scopes
v8::Isolate::Scope isolateScope(rt->isolate());
v8::HandleScope handleScope(rt->isolate());
v8::Context::Scope contextScope(rt->context());

// Evaluate a module
// 当前工作目录取决于 property runtime.working-path，可在命令行中直接指定
v8::Local<v8::Value> ret;
if (!rt->evaluateModule("index.js").ToLocal(&ret))
{
    // Error
}
// 运行主事件循环，主要用于处理 JavaScript 中的异步 Promise
// 用户注册的一些回调函数也要在事件循环中处理
EventLoop::Ref().run();
```

Cocoa 一旦开始运行，在进行完必要的初始化工作后，便会在工作目录
（对应 property `runtime.working-path`）查找文件 `index.js`，若找到便开始执行，
若没有找到则抛出异常，并以错误状态退出。由此可见，Cocoa 直接加载 JavaScript 脚本，
但 JavaScript 并不是 Cocoa 的首选脚本语言。TypeScript 是一种将 JavaScript
作为目标语言的编译型语言，相比 JavaScript，它提供了更严格的类型系统（强类型语言），
更易于维护和阅读，因而 Cocoa 选择 TypeScript 作为官方脚本语言（标准库是 TypeScript
编写的）。

Cocoa 支持 __ECMAScript 6 (ES6)__ 标准，与 Node.js 不兼容（CommonJS 不兼容）。

## Language Binding
__语言绑定（Langugae Binding）__ 用于在 JavaScript 中发起对原生代码的调用。
大多数情况下，原生代码由 C++ 层实现，借助 `src/Koi/binder` 下的一些模板元编程技巧
（来自 [Github 上的 v8pp 项目](https://github.com/pmed/v8pp)），
我们可以很方便地把一个 C++ 类或函数绑定到 JavaScript 中。下面的示例将一个 C++ 类
`Test` 绑定到一个 JavaScript 对象上：
```cpp
class Test
{
public:
    explicit Test(const std::string& str);
    // JS 中的对应对象被 GC 回收或 Runtime 结束运行时
    // 将调用析构函数，同时释放 C++ 对象。
    // 析构函数中 V8 的所有 API 均可用。
    ~Test();

    void foo();
    int bar(bool val);
};

auto klass = binder::Class<Test>
             .constructor<const std::string&>() // 注册（唯一）构造函数签名
             .set("foo", &Test::foo)            // 注册成员函数 foo
             .set("bar", &Test::bar);           // 注册成员函数 bar

binder::Module mod;
mod.set("Test", klass);

// 在 JS 中对该对象的 Test 成员调用 new Test(...) 时，
// C++ 中对应的 Test 类被构造。
v8::Local<v8::Object> jsObj = mod.new_instance();
```
为了维护不同的语言绑定，我们设计了一系列机制来统一语言绑定的接口，
所有关于语言绑定的类和函数等都位于 `cocoa::koi::lang` 命名空间下。

`lang::BaseBindingModule` 类表示了一个独立的语言绑定，
任何语言绑定都至少应该继承该类，并在子类中实现方法 `void getModule(binder::Module&)`，
在基类构造函数中填写语言绑定的名称字段 `name` 以及描述字段 `desc`，
前者决定了语言绑定在 JavaScript 中所呈现的对象名（TypeScript 中的命名空间）。
例如，`name` 字段为 `foo` 的语言绑定在 JavaScript 对应的对象名为 `Cocoa.foo`。
而该语言绑定的 `desc` 字段在 JS 中可以通过 `Cocoa.foo.__desc__` 访问。

`lang::BaseBindingModule` 有一个特殊的子类 `lang::BindingChain`，该类没有在头文件中定义，
属于 Cocoa 的内部类型。Cocoa 总是维护一个 `gChain` 全局变量，类型为
`lang::BindingChain`。与此同时，每个 `lang::BaseBindingModule`
都有一个 `next()` 方法，该方法返回所谓的 next 指针，即指向下一个 `lang::BaseBindingModule`
的指针。这便形成了一种类似于链表的链式数据结构，
每个成员都持有指向下一个成员的指针，我们称为 __语言绑定链(Language Binding Chain, LBC)__。
而 `gChain` 则是整个 LBC 的首端，但它不表示任何有效的语言绑定，
或者说它是一个空语言绑定，真正的语言绑定（即一个 `lang::BaseBindingModule`）
应该从 LBC 的第二个元素开始。

`lang::RegisterBinding` 函数可以将一个 `BaseBindingModule`
指针添加到 LBC 尾部。

## Development Environment
在 `typescript` 目录下有 Cocoa 项目使用到的所有 TypeScript 文件，包括标准库。
每一个子目录都对应一个 Language Binding。
