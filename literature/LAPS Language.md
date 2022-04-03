# LAPS: Layer Paint Script Language Introduction

## Basic Concepts
### 1. Statement（语句）
LAPS 是结构化编程语言，语句是其基本组成单位。同 C 语言一样，任何 LAPS 语句由分号
（;）结束，对于块声明（见下文），无需分号结尾。LAPS 语句分为两种，
一是用于表示控制流程的语句，他们包括分支、选择、循环、赋值、表达式等，
这些语句是 **无副作用（Side Effect）** 的，
这意味着它们本身不会对翻译单元（Translation Unit）外的对象起作用
（一些函数调用语句可能包含对翻译单元外的对象的操作，
但那不应被视为语句本身的副作用，而是函数的副作用）。
另一类语句是用于表达渲染指令的 **绘制指令（Drawing Operations）**，
绘制指令直接作用于该 LAPS 翻译单元所依附的 `Layer` 对象，生成 `SkPicture`
对象。绘制指令由谓词和参数构成，**谓词（Verb）** 直接表明绘制指令的主体，
通常是表达一个最小片元（直线、多边形等）；
而 **参数（Arguments）** 是对谓词的补充，通常指定了片元的坐标、颜色等。

```
// Assignment
let pos: float2 = float2(20.0f, 20.0f);
// Drawing operation
@point pos;
```

### 1. Context Scope（上下文作用域）
在 LAPS 中，绘图指令需要指定绘图目标，这称之为它的 **上下文（Context）**。
