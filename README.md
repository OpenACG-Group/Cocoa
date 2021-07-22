Cocoa
=====
*****

Cocoa 是高性能、现代化的通用 2D 渲染引擎。

Architecture
------------
Cocoa consists of different parts:
* **Core** provides basic functions for other parts.
* **Vanilla** is an abstraction layer of window systems, and a hardware-accelerated
  2D rendering engine based on Google's Skia.
* **Komorebi** is an JIT implementation of Vanilla Shading Language (VSL) base on LLVM.
* **Scripter** Executes JavaScript, and provides an runtime environment like Node.js.

JavaScript in Cocoa
-------------------
Many modern operating systems divides the whole system into userspace and kernel-space.

Animations
----------
Cocoa can load a Lottie JSON file for playing animations. Lottie files can be exported by
*Adobe After Effect* or other similar tools.

Komorebi Extensions
-------------------
Cocoa supports many interesting and beautiful visual effects. They're provided as extensions of
**Komorebi**.

To load an extension in Javascript:
```javascript
let live2d = window.require("CubismLive2DWidget");
```

### KaleidoscopicText Widget
For text rendering, **Kaleidoscopic Text Description Language** or **KTDL** can be used.
KTDL is a markup language that focuses on the display and layout of text.
KTDL allows the user to specify a series of attributes for a text that will be applied
to the text within its scope.

Following chart shows some useful attributes in KTDL.

|     Attribute/Declaration        |    Shorthand     | Description |
|----------------------------------|------------------|-------------|
| [n!]                             | [n!]             | New line |
| [t!]                             | [t!]             | Tab character (8 spaces is the default) |
| [set-tab *integer*]              | [stab *integer*] | Set spaces for a tab character |
| [font-size *integer*]            | [ftsz *integer*] | Specify font size (pixels) |
| [font-style Normal,Italic,Bold]  | [ftst N,I,B]     | Specify font style |
| [font-weight Normal,Bold,Light]  | [ftw N,B,L]      | Specify font weight |
| [font-name *string*]             | [fn *string*]    | Specify font name |
| [foreground *hex color*]         | [fg *hex color*] | Specify foreground color (font color) |
| [background *hex color*]         | [bg *hex color*] | Specify background color |
| [link *URI*]                     | [link *URL*]     | Hyperlink |
| [translate *content*]            | [trl *content*]  | Display the translation above the text |
| [black-mask]                     | [bmask]          | A black opaque mask appears on the text |
| [underline]                      | [ul]             | Render underline |
| [strikeout]                      | [sto]            | Render strikeout |

The following example uses KTDL to display the corresponding Hiragana on Japanese Kanji characters.
<pre style="font-family: Consolas, sans-serif; font-size: 14px">
[ftsz 15]
[trl みらい]未来[!trl]の[trl まえ]前[!trl]にすくむ[trl てあし]手足[!trl]は[n!]
[trl しず]静[!trl]かな[trl こえ]声[!trl]にほどかれて[n!]
[trl さけ]叫[!trl]びたいほど　なつかしいのは[n!]
ひとつのいのち[n!]
[trl まなつ]真夏[!trl]の[trl ひかり]光[!trl][n!]
あなたの[trl かたに]肩[!trl]に　[trl ゆれ]揺れ[!trl]てた[trl こもれび]木漏れ日[!trl][n!]
[!ftn]
</pre>

### Cubism Live2D Widget
TODO: Complete this.
