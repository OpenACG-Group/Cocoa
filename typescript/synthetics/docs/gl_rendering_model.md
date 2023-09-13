Glamor Rendering Model
======================

## Introduction
Glamor is the 2D rendering engine (3D might be supported in the future) of Cocoa,
based on Google's graphics library Skia. Glamor provides a set of imperative programming
style APIs for rendering, with some declarative DSLs like the filtering DSL.
This tutorial involves the basic architecture and API usages of Glamor engine,
providing users a simple way to understand its design concepts.

`glamor` module can be imported by:
```typescript
// `synthetic://glamor` is also an acceptable module name
import * as gl from 'glamor';
```

## Draw Commands
When it comes to rendering, the first problem you think might be: how can we let
the computer know what we want to draw?

As we have mentioned above, Glamor provides a set imperative rendering APIs, which
means that you should tell the engine how to draw a picture in detail, instead of
simply describing what the picture looks like. In other words, you give the engine
__"draw commands"__ to instruct it on rendering a picture. For example, the following
pseudocode is a list of draw commands:

```
Clear
DrawCircle      center=(10, 10),r=10
DrawRectangle   x=0,y=0,w=100,h=100
...
```

We can transcribe it into TypeScript code:
```typescript
const renderer = new Renderer();
renderer.clear();
renderer.drawCircle(10, 10, 10);
renderer.drawRectangle(0, 0, 100, 100);
```

This programming style is called __imperative programming__.
Each function call like `clear()` or `drawCircle()` is equivalent to create a corresponding
draw command. And that's the reason why their function names are verbs or verb-object
phrases. To describe that process more technically, we say the `renderer` object __generates__
or __emits__ the draw commands, so the `renderer` object itself is a draw command __generator__
or __emitter__.

Imperative programming is commonly used in computer graphics, like
[Web Canvas API](https://developer.mozilla.org/en-US/docs/Web/API/Canvas_API).
Another widely used programming style in 2D rendering is declarative programming,
which usually use markup languages to describe the contents of a picture.

## Canvas API

### Render Target
When we talk draw commands, there must be a certain _render target_ on which we do
the drawing operations. More exactly, the render target is a renderable object
__where the draw commands draw things__.

Imagine that you're a painter, your brain generates draw commands, and the canvas is your
render target. But the problem is that you're a skillful painter who can draw pictures
on many different surfaces like normal papers, walls, roads, etc. It is obvious that you
should use different tools while drawing on different surfaces, for example, small brushes
when drawing on papers or spray cans when drawing graffiti on walls.
__The drawing tools change with different render targets.__
That's true of computer rendering. Glamor supports many different render targets, including
pixel buffer stored in memory, GPU textures, SVG pictures, and so on.
Different render targets using different methods of rendering means that it will be painful
when you want to change your code to support another render target. You have to rewrite
all the rendering-related code.

Go back to the example of a painter above, and we can find that although the painter uses
various tools to adapt for different surfaces, __the draw commands is exactly the same
as long as you're drawing the same picture__. Similarly, for Glamor, no matter what
the render target is, as long as you're drawing the same picture, the draw commands are 
exactly the same. That's what the __Canvas API__ does. It gives you a general API to
generate draw commands, allowing you not to care about the differences among render
targets. Your draw commands will be translated into a suitable representation that fits
the current render target.

Try to run the following code:
```typescript
import { CkImageInfo, CkSurface, CkPaint, Constants } from 'glamor';
import { File, Buffer } from 'core';
import { PNGEncoder } from 'pixencoder';

// Create a `CkSurface` object, which is the render target.
// `MakeRaster()` creates a pixel buffer render target.
const imageInfo = CkImageInfo.MakeN32Premul(256, 256);
const surface = CkSurface.MakeRaster(imageInfo);

// Get the Canvas API from render target.
const canvas = surface.getCanvas();

// Draw something on the canvas.
// The `paint` object specifies some drawing parameters.
// See the API document for more details.
const paint = new CkPaint();
paint.setStyle(Constants.PAINT_STYLE_STROKE);
paint.setColor4f([0, 0, 1, 1]);
paint.setAntiAlias(true);
paint.setStrokeWidth(3);
canvas.clear([1, 1, 1, 1]);
canvas.drawCircle(100, 100, 50, paint);

// Save the image
const encoded = PNGEncoder.EncodeImage(surface.makeImageSnapshot(null), {});
File.WriteFileSync('output.png', Buffer.MakeFromAdoptBuffer(new Uint8Array(encoded)));
```

Open `output.png` file, and you will see a blue circle.

This example shows you how to use the Canvas API. `canvas` is an instance of `CkCanvas`
class, but you must not use `CkCanvas` constructor to create a canvas object
(you'll get an exception if you do that).
As we showed above, only render targets can create canvas objects. The created canvas
draws things on the render target that creates it.

### CkPaint
Some Web programmers may be familiar with the canvas API, as the concept of "canvas"
is also used in Web development, and HTML5 provides a canvas API to manipulate the
`<canvas>` element. However, Glamor's canvas API is quite different from the browser's.
The most obvious difference is that Glamor's canvas API is __stateless__ except clipping
and matrix, while browser's canvas API is __stateful__ for all the drawing states, including
color, style, stroke-width and so on.

Browser's canvas API is usually used like:
```javascript
const ctx = canvas.getContext("2d");
ctx.fillStyle = "rgb(200, 0, 0)";
ctx.fillRect(10, 10, 50, 50);
ctx.fillStyle = "rgba(0, 0, 200, 0.5)";
ctx.fillRect(30, 30, 50, 50);
```
The `fillStyle` is stored in canvas context itself, and will be used when needed.

Glamor's canvas API is used like:
```typescript
const paint = new CkPaint();
paint.setStyle(Constants.PAINT_STYLE_STROKE);
paint.setColor4f([0, 0, 1, 1]);
paint.setAntiAlias(true);
paint.setStrokeWidth(3);
canvas.drawCircle(100, 100, 50, paint);
```

Drawing states are stored in an external object `paint`, and you should provide that
object to canvas whenever it is needed. After `drawCircle` returns, required datas
have been copied from `paint` to the generated draw command, which means you can change
`paint` object without affecting what you have drawn.

### CkPath
A path is a group of closed or not closed __contours__. Contour is an outline __which
consists of one or more segments, including curves or lines, connected end-to-end__.
Specially, a single point is considered as an empty contour. If the start point and the
end point of a contour are connected, forming a loop, it is a __closed__ contour.
Circles, straight lines, any polygons all can be considered as contours.
In computer graphics, we can simply say that contour is an outline that we can stroke
along with.

To create a path, a `CkPath` object should be constructed.
`CkPath` is a container of contours:
```typescript
const path = new CkPath();
```

Add some contours:
```typescript
// First contour, a circle
path.addCircle(100, 100, 50, Constants.PATH_DIRECTION_CCW);

// Second contour, a triangle
path.moveTo(10, 10);
path.lineTo(20, 20);
path.lineTo(5, 30);
path.close();
```

`moveTo()` moves the "current start point" to the specified position.
`lineTo(x, y)` connects the current start point with the specified point (x, y) by a
straight line, and moves the current start point to (x, y). See the API documentation
for more details about other methods of `CkPath`.

The created path can be drawn directly:
```typescript
canvas.drawPath(path, paint);
```

You also can examine a path by using `CkPathMeasure` class. It involves some complex
calculation in computing geometry: lengths of segments, the unit tangent vectors, etc.
That information can be useful to implement complicated effects, like drawing texts
along with a contour.

### Clipping and Matrix


### Save and Restore


## Recording and Deferred Rendering

### Deferred Rendering

### CkPicture and CkPictureRecoder

## Onscreen Rendering

### Frame Timing

### Paint, Submit, Present