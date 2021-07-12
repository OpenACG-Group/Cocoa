# Command Specification (Chinese)

## Introduction
通过命令行启动 Cocoa 时，可以通过一些命令行选项和参数改变 Cococa 的行为，本文档描述了
这些选项和参数，以及它们的作用和使用规范。
关于术语选项（Options）和参数（Arguments），我们将在下文中区分。

## Basic Specification
Cocoa 的命令行语法基本沿用 GNU/POSIX 规范，也就是大部分 Unix 程序采用的规范，但在一些细节
上有所更改，用于消除二义性。

### Options and Values
__选项（Options）__ 控制程序的行为，从语义层面来讲，选项本身可以看作是谓词或具有谓词性质的
短语，它描述了主体（程序）与可选的客体（选项的值）之间的关系。
我们把选项大体分为如下几种：

__短选项（Short Options）__：短选项以短横线 `-` 开头，后面可以接一个拉丁字母，短横线仅仅
作为语法上的区分，避免混淆命令行的选项和参数，后接的单个字符才具有语义上的意义，表示用
户具体希望指定什么选项。例如，`-a` 是一个选项，a 可能是某个具体谓词的缩写。

__带值短选项（Short options with value）__：带值短选项是单纯的短选项的延伸，在短选项后跟上
一个或多个空格等分隔符（具体的分隔符取决于用户使用的 Shell），然后紧跟该选项的值。该值
描述了选项具体起作用的对象或者是对选项的补充说明。例如 `-v 1.0` 是一个带值短选项，它表示
选项 v (velocity) 具有一个值 1.0，这可能表示我们希望指定某种速度为 1.0 m/s.

__复合短选项（Complex Short Options）__：复合短选项是多个短选项的一种简写形式，当多个不带
值短选项连续出现时，我们可以去掉它们开头的短横线，把所有选项字符写在一起并作为一个整体在其前面
添加一个短横线，用以压缩命令行的长度和增强可读性。例如，多个短选项 `-a -b -f -v` 可以简写为
`-abfv`。当复合选项后面带值时，我们就称其为带值复合短选项，并且遵循就近原则，即该值属于复合短
选项的最后一个选项。例如 `-abfv enable`，值 `enable` 属于 `-v` 选项。

上述的三种短选项，我们统称为 POSIX 风格选项，Cocoa 完全支持。

__长选项（Long Options）__：长选项以两个短横线 `--` 开头，两个或两个以上的由拉丁字符、数字、
下划线、短横线组成的字符串。不同于短选项采用缩写，长选项用完整的单词描述了选项的功能，因此
可读性更强，但同时增加了命令的长度。一些较为复杂的选项，或者在缩写上由冲突的选项，我们通常会使用
长选项。例如 `--disable-dbus-service` 是一个长选项。程序可能会同时提供具有相同语义的长选项
和短选项，此时可以根据具体情况进行选择。例如 `--help` 和 `-h` 具有相同的功能。

__带值长选项（Long options with value）__：和短选项一样，长选项也可以带值，长选项的值通过在
选项后紧跟一个等于号并在等于号后直接书写具体值的形式来指定。例如 `--config=/path/to/config.json`.

上述两种长选项，我们统称为 GNU 风格选项，Cocoa 完全支持。

__Short options with value attached__：在短选项后直接跟上它的值，如 `-O2`。这种语法 Cocoa 不支持。

### Supported Options
<pre style="font-family: Consolas">
-c, --config <file> Specify the JSON configuration file.
</pre>

## Command Overrides
Command Overrides 的概念基本沿用自 JSON，例如对象和数组。不同的是，我们不允许
对象或数组的嵌套，一个 override 只能设置一个基本类型值。总的来说，Command
Overrides 具有如下限制：
* 不允许对象的嵌套；
* 不允许元素类型为复合对象的数组；
* 不允许元素类型不同的数组。

### 词法
Command Overrides 接受标准的 ASCII 字符、句点、冒号、开闭方括号以及逗号，对于
值中的字符串字面量，接受标准的 UTF-8 编码字符串。

### 形式语法
一个 Command Override 具有如下典型形式：<br/>
    `<Property Specifier>: value`                   (对于一般字面量)<br/>
    `<Property Speicifer>: [value1, value2, ...]`   (对于数组)<br/>

### 严格语法
对于任意的合法 Command Override 语句，定义如下终结符：<br/>
对于形如 "true" 的串：`TRUE`<br/>
对于形如 "false" 的串：`FALSE`<br/>
对于任何非数字开头，仅包含数字、大小写拉丁字母，且不是上述任何情况的串：`IDENTIFIER`<br/>
对于任何仅包含数字、可选地以 "-", "+" 开头的串：`INTEGER`<br/>
对于任何仅包含数字、句点且可选地以 "-", "+" 开头的串：`FLOAT`<br/>
对于任何包含在双引号内（除开有转义符号 "\" 引导的双引号）的串：`STRING`<br/>

则有如下非终结符：<br/>
```
statement → property_specifier : value

value → pure_value
        | [ value_list ]

value_list → pure_value
             | value_list ,

pure_value → TRUE
             | FALSE
             | INTEGER
             | FLOAT
             | STRING

property_specifier → IDENTIFIER
                     | property_specifier .
```
其中，statement 是起始符号。
