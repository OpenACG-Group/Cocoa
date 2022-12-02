Vizmoe: A Reference Implementation of Visual Novel Engine
=========================================================

Vizmoe provides a reference implementation of visual novel engine
based on Cocoa rendering framework. *Viz* is an abbreviation of the Latin
word *videlicet*, meaning "it is permitted to see", and *moe* stands for
the pronunciation of the Japanese slang *萌え*, which is widely used in
anime and manga. The combination of the two words roughly describes what
*visual novel* means.

Scenario
--------

Vizmoe is basically a __scenario parser and layout engine__ designed for
visual novel applications. Users should provide at least one scenario file
and describe what to display in that file in __VKS language__.
VKS means __Visual Kinetic Scenario__ as kinetic novel is a type of
visual novel.

Here is a simple example for VKS langauge:

```
# Comments start with a "#"
# Filename: startup.vks

@drawing_primitives []() {
}
```
