#!/usr/bin/env bash

#function msg() {
#    echo -e "\e["
#}

declare cr_title1="\e[34;1m"
declare cr_title2="\e[33;1m"
declare cr_reset="\e[0m"
alias out="echo -e"

## Clear screen first
clear

function wait_for_enter() {
    read
    clear
}

echo -e "
${cr_title1}[*] I. Prologue${cr_reset}
            ${cr_title2}Cocoa JavaScript Language Binding Tutorial${cr_reset}
                    OpenACG Group - Cocoa Project
                            2022-02-03

该工具是一个交互式的 Cocoa 语言绑定编写指南，该指南会分步地指导您如何编写规范的
Cocoa 语言绑定并以正确的方式加载、使用它们。
您只需仔细地阅读提示，然后按照指示动手操作，很快便能编写出一个基本的 Cocoa 语言
绑定程序。
在开始之前，请确保您熟悉以下概念：
    - C++ 语言
    - Cocoa 的基本设计理念
    - Linux 平台下的动态链接库（Shared Object, 共享对象）文件
    - 二进制编程接口（ABI），共享库中符号的概念
    - C/C++ 语言的编译、链接过程
    - C++ 编译过程中符号 mangle/demangle 的概念

确保这些前置知识准备充分之后，我们就可以开始编写 Cocoa 语言绑定的愉悦旅程了。
在继续之前，您需要明知几点：
    (1) 任何时候，如果屏幕上的文字不再更新（或者没有要更新的迹象），按下 ENTER 翻页；
    (2) 任何时候，如果出现类似 \e[1mfile:///path/to/file\e[0m 的 URL，如果您的终端
        支持，按下 Ctrl 并点击可以直接打开 URL 指向的网页或文件（使用默认的处理程序）；
    (3) 任何时候，若非上下文语境中指明，「本目录」皆是指 file://$(pwd) 目录。

按下 ENTER 继续...
"
wait_for_enter

echo -e "
${cr_title1}[*] I. File Structure${cr_reset}
该目录下包含了一个名为 ATRI 的最简语言绑定，但它包含了创建一个语言绑定所需的全部要素
和编程技巧，因此是一个相当好的示例。
PS：ATRI 这个名称来自于 ANIPLEX.EXE 发售的视觉小说 \e[3;1mATRI -My Dear Moments-\e[0m，
这也是 Cocoa 项目最初的灵感来源。

本目录下有许多文件，而 ATRI 子目录下则是示例语言绑定的源文件。
现在您需要对它们的功能有一个大致的了解，要在短时间内简要清楚地说明 Cocoa 语言绑定的运
行机制是十分困难的，这是一个比较复杂的系统。因此，我们只会简单地介绍如何使用它提供的
API，而不对其原理作深入的阐述。

目录是语言绑定的基本单位，虽然不是必须，但是我们强烈建议您将您的语言绑定的所有相关文件存放到
一个目录中，而目录名即语言绑定名。

若您使用 IDE，请为 IDE 配置如下 C++ include 文件查找目录，以便 IDE 能正确找到所需的头文件：
（<Project root> 指项目根目录）
    - \e[3;1m<Project root>/src\e[0m
    - \e[3;1m<Project root>/third_party/v8\e[0m
    - \e[3;1m<Project root>/third_party/v8/include\e[0m

现在，请大概浏览 ATRI 目录，里面只有三个必须的文件，随便看看它们包含的内容，有兴趣的话，最好
是自己试着根据代码中的命名和文件名，推测一下其中的类和函数的作用。

阅读完成后，按 ENTER 继续...
"
wait_for_enter

echo -e "
${cr_title1}[*] II. Concepts${cr_reset}
现在我们需要先来了解一下 Cocoa 语言绑定的相关概念。JavaScript 是基于原型的面向对象，因此对于下文中提到
的 class, interface 等术语，参照 TypeScript 的定义。

\e[32;1m模块（Module）：\e[0mES6 标准中引入的「模块」概念。

\e[32;1m导出（Exports）：\e[0m模块暴露给外部的所有可访问符号的集合。而顶层导出是指您
使用 ES6 的 import 语法可以直接访问到的导出。它们在源代码顶层被定义，而不是封装在 class 或 interface
内。例如，test 模块导出了一个 Test 类，而 Test 类中有方法 doTest。那么，Test 就是一个顶层导出（Toplevel Export），
doTest 则不是，因为它需要 Test 类才能被访问到。

\e[32;1m合成模块（Synthetic Module）：\e[0m不包含\e[1m任何 JavaScript 代码\e[0m，所有的导出全部是
\e[1m来自语言绑定的原生代码\e[0m的模块。所有的语言绑定都被 Cocoa 抽象封装为合成模块，可以以通用 ES6
语法来导入它们。

经过前一页的大致浏览和探索，您应该已经对语言绑定的源代码结构有了大致的了解。
其中最让您感到困惑的文件想必是 \e[33;1mexports.mds\e[0m 了，正如其名，该文件定义了语言绑定的「导出」。当然，
这里的导出是指顶层导出。由此自然想到，对于每一个独立的语言绑定，应该有唯一的 exports.mds 文件与之对应。

实际上，exports.mds 这样的文件称为模块符号描述文件（Module Description of Symbols）。

现在，请您打开 exports.mds 文件，与之前不同，这次请在了解上述定义后，仔细观察其中的内容，并依据名称推断
每一行的意义。

阅读完成后，不要关闭文件，按下 ENTER 继续...
"
wait_for_enter

echo -e "
${cr_title1}[*] III. Module Description of Symbols${cr_reset}
您应该注意到了，exports.mds 文件的每一行大部分都以 % 开头，后面紧跟一个标识符，有的还连缀一些参数。
那么，我们将向您介绍 MDS 基本语法：

\e[33;1m指令：\e[0m以 % 开头的行称为指令，指令的基本格式是 \e[35;1m'%IDENT [ARG1 ARG2...]'\e[0m。其中 IDENT
部分称为谓词，谓词是指令的主体，指导 MDS 的解析器如何解释这条指令，或者用更自然的语言描述，谓词像是
在回答「这条指令要干什么？」这个问题。之后可以接一个以空格分隔的参数列表，具体的参数数量和意义由谓词
决定。参数是对谓词的补充，起修饰限定作用，通常，它指定谓词的支配对象。稍后将介绍常见的谓词以及它们的
参数。

\e[33;1mCDATA：\e[0m没有以 % 开头的行全部是 CDATA（空行除外），CDATA 只能在特定的位置出现，通常是
由特定的指令限定的区域。

\e[33;1m谓词对：\e[0m正如其名，谓词对是需要配对使用的一对谓词，所有的谓词对都具有 _begin 或 _end
后缀。谓词对最常见的用法是用于指定一段 CDATA。

下表列出了 exports.mds 文件中出现的谓词以及对应的含义（: 表示谓词对）：
    \e[32;1mheader_begin:header_end\e[0m
        该谓词对之间插入一段 CDATA，表示一段插入头部的 C++ 代码，稍后您会了解到这句话的含义。
    
    \e[32;1mnamespace\e[0m
        指定该语言绑定的 C++ 命名空间。
        ARG1 - NAMESPACE [必须] 命名空间名
    
    \e[32;1mclass\e[0m
        指定该语言绑定的绑定类，绑定类的概念稍后介绍。
        ARG1 - CLASS [必须] 不含有命名空间的类名。该类必须定义在 namespace 谓词指定的命名空间下。
    
    \e[32;1mexport\e[0m
        指定一个导出
        ARG1 - DECL [必须] 导出在 JavaScipt 中的符号名称
        ARG2 - EXPR [必须] 一个合法的 C++ 表达式，该表达式的结果必须是左值。表达式的结果就是导出
                           到 JavaScript 的值。C++ 基础类型想必您没有什么疑问，至于字符串，函数，
                           类一类的值如何导出到 JavaScript，稍后会介绍。

对照您打开的 exports.mds 文件，根据上述定义，尝试解释每一行的含义。

按下 ENTER 继续...
"
wait_for_enter


echo -e "
${cr_title1}[*] IV. C++ Definitions, Declarations${cr_reset}
像上文提到的那样，我们将介绍「绑定类」的概念。从此处开始，我们将进入熟悉的 C++ 世界。

绑定类是一个 C++ 类，它必须继承自 BindingBase 类，而一个绑定类则唯一代表了一个语言绑定。绑定类的一个
实例就代表了一个语言绑定实例，\e[1m同一个 Cocoa 进程中，同一个语言绑定只会有一个实例\e[0m。

绑定类的作用主要在于，作为语言绑定的代表，对外提供一些 Cocoa 加载语言绑定是必须的方法，另外，绑定类还
可以兼顾一部分语言绑定的资源管理工作，因为绑定类会在语言绑定被加载时构造，在语言绑定被卸载时析构。
再另外，绑定类还与 MDS 文件相接应。

打开 atri.h 文件，您应当会发现定义了 AtriBinding 类。阅读对应的注释，跳转到 BindingBase 类，您
还可以阅读在 BindingBase 中的注释来了解一些细节。

\e[34;1m现在您希望...\e[0m
    > \e[35;1m[1]\e[0m 我自己来！
    > \e[35;1m[2]\e[0m 你来帮我运行了吧……"
printf ">> \e[33;1mSELECT ([1]/2): \e[0m"
read
wait_for_enter
