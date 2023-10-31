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

## Symbols

This article uses the following symbols to indicate basic geometry objects:

* $(x,\;y)$ A two-dimensional vector $\bm{v}$ represented by coordinates.
  $\bm{v} = x\bm{i} + y\bm{j}$, where $\bm{i}=(1,0),\;\bm{j}=(0,1)$ is a pair
  of orthonormal bases.
* $Rect(x,\;y,\;w,\;h)$ A rectangle with vertical and parallel sides.
  The top-left corner is at $(x,\;y)$, and the right-bottom corner is at
  $(x+w,\;y+h)$.

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
along with a contour. To measure a path, create a `CkPathMeasure` instance first:

```typescript
const measure = CkPathMeasure.Make(path, /* forceClose */ false, /* scalar */ 1);
```

The `measure` object can be treated as an iterator of contours contained in `path`.
Use a while-loop to iterate on contours:

```typescript
// `nextContour()` returns false when there are no more contours.
while (measure.nextContour()) {
  evaluateCurrentContour(measure);
}

function evaluateCurrentContour(measure: CkPathMeasure): void {
  // Get the length of current contour, in pixels
  const L = measure.getLength();
  // Examine points on the curve.
  for (let t = 0; i <= 1; t += 0.1) {
    // Compute the position p and the unit tangent vector v
    // of the specified point.
    const [p, v] = measure.getPositionTangent(/* distance */ L * t);
    // ...
  }
}
```

For the simplest case, supposing the contour is a smooth curve

$$
\begin{cases} x = \varphi(t) \\ y = \phi(t) \end{cases}, t\in[0, 1],
$$

`getLength()` returns the length of that curve, which is defined as

$$
L = \int_{0}^{1} \sqrt{[\varphi'(t)]^2 + [\phi'(t)]^2} \mathrm{d}t,
$$

and `getPositionTangent(d)` returns the position $\bm{p}$ and the unit tangent vector $\bm{v}$
of that curve where $\displaystyle t = \frac{d}{L}$. Vector $\bm{p}$ and $\bm{v}$ is defined as:

$$
\bm{p} = (\varphi(t),\;\phi(t))
$$

$$
\bm{v} = (
  \frac{\varphi'(t)}{\sqrt{[\varphi'(t)]^2 + [\phi'(t)]^2}},\;
  \frac{\phi'(t)}{\sqrt{[\varphi'(t)]^2 + [\phi'(t)]^2}})
$$

It is obvious that $|\bm{v}| = 1$. The angle between $\bm{v}$ and x-axis or y-axis can be
computed by

$$
\begin{cases}
  cos\theta_x = \bm{v} \cdot \bm{i} \\
  cos\theta_y = \bm{v} \cdot \bm{j}
\end{cases}
$$

which is useful for drawing something like texts along the path.

### Clipping

A clip region is defined so that __the pixels are only drawn in the specified boundaries.__
Pixels out of the clip region won't be touched, although they are supposed to have been
changed by draw commands. For example:

```
SetClipRect     Rect(10, 10, 100, 100)
DrawLine        (0, 0), (300, 300)
```

Command `DrawLine` __should have drawn__ a line from $(0,0)$ to $(300,300)$. However,
with a clip region $Rect(10, 10, 100, 100)$ set, only the part where the line intersects
with the clip region will be drawn, which is a line from $(10,10)$ to $(100, 100)$.

The clip operation described above is called __IntersectClipOp__. Besides, there is another
clip operation called __DifferenceClipOp__. When it is applied, only the part that
__does not intersect with the clip region__ will be drawn. For the example showed above,
two segments, one from $(0,0)$ to $(10,10)$ and another from $(100,100)$ to $(300, 300)$,
will be drawn.

Clip is set on the canvas directly:

```typescript
// Clip region can be a rectangle or round-rectangle.
// The last boolean argument controls whether to apply antialias for clip region.
canvas.clipRect([10, 10, 100, 100], Constants.CLIP_OP_INTERSECT, true);
// Clip region also can be a path
canvas.clipPath(path, Constants.CLIP_OP_INTERSECT, true);

// Clip region also can be a shader, see Skia API for more details.
```

Canvas provides a way to quickly (and roughly) determine a rectangle or path is
outside of current clip:

```typescript
// Note that this method does not involve accurate intersect test.
// It may return false even though the shape is outside of clip,
// but it must return false when the shape is inside of clip.
if (!canvas.quickRejectPath(path)) {
  // `path` may be inside of clip, and should be drawn.
  canvas.drawPath(path);
} else {
  // `path` is outside of clip, and we can skip drawing it.
}
```

### Matrix

### Save and Restore

### Shader

### ImageFilter and ColorFilter

### RuntimeEffect

## Recording and Deferred Rendering

### Deferred Rendering

### CkPicture and CkPictureRecoder

## Onscreen Rendering

### Frame Timing

### Paint, Submit, Present

