# Command Overrides Specification (Chinese)

## Introduction
通过命令行启动 Cocoa 时，允许临时更改一些 JSON 中的配置选项。这一功能通过
Command Overrides 实现。要更改配置，在命令行选项中添加 `--override=<spec>`
或 `-o <spec>`。

## Specification
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
