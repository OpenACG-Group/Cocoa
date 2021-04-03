# Ciallo 2D Rendering Engine
<span style="font-family: Georgia, serif; font-style: italic; font-size: medium">
Developer's Manual Chapter II
</span>

---------------
## Introduction
__Ciallo__ is a platform-depended 2D rendering engine. It provides a
platform-independed interface for rendering both UI and complex images.

## Window System Integration
Ciallo should be used with a certain window system to display.
The following window systems are supported now:
* X11 (Requires XRender extension for UI rendering)

For each supported platform, we hold a context of this platform. For example,
XcbConnection is the context of X11 platform. Then we can create windows and
receive input events by the context. "Window" is also abstracted into an object,
call the methods to manipulate the window (resize or replace, set title and icon...).

Besides, each window can be treated as a __Drawable__ object. The rendering backends
accept a __Drawable__ object as the destination of drawing. All of the events
on a certain Drawable will be handled by its __DrawableListener__. Following code
shows this:
```c++
class MainWindowListener : public DrawableListener
{
public:
    MainWindowListener(...);
    ~MainWindowListener() override;
    
    void onRender() override
    {
        // Render something...
    }
    
    void onDestroy() override
    {
        // This method will be called before a window
        // being destroyed.
    }
    
    bool onClose() override
    {
        // This method will be called when a window
        // is closed by user.
        // Return true if the window can be closed.
        return true;
    }
    
    // Other events like keyboard and mouse inputs...
};

// ......
// Create a Drawable (window) and attach a listener
XcbConnection connection;
XcbWindow window(connection.screen(), SkIRect::MakeXYWH(0, 0, 800, 600));

auto listener = std::make_shared<MainWindowListener>();
window.setListener(listener);

// ......
```

Note that the drawing operations must be performed in a special period of time,
we call it __Exposure__. Only the drawing operations performed at the time of
Exposure will be actually displayed on the window. That means, all the drawing
operations must be performed in `DrawableListener::onRender()` (Skia2d with
GPU acceleration is a special case, you can update the window at any time,
see "Rendering Backends" below).

### Event Loop in Window System
All of the events which are received by windows will be handled by __EventDispatcher__
in __Core__ module. See also [Core Module](Core.md).

## Rendering Backends
Ciallo abstracts two rendering interface, Cairo2d and Skia2d. As you can see,
they use Cairo and Skia (2D graphics library) as their backends, respectively.
The graphics pipeline of them may be totally different.

### Cairo2d
The __Cairo2d__ interface is mainly used to render the internal UI. In most case,
it  renders content by the window system or display server directly. For an example,
Cairo2d uses __XRender__ on X11 platform, the rasterization is completed by
X11 server. The source code is in `src/Ciallo/Cairo2d`.

Here is an example for using Cairo2d backend:
```c++
using namespace cocoa::ciallo;

// Create a surface to render
// Use CrSurface::MakeFromDrawable to render into a window.
// The CrSurface object is reference-counted. It will be destroyed
// before leaving the scope if the reference count is zero.
auto surface = CrSurface::MakeImage(250, 250);

// CrCanvas is very similar to a state machine, it stores
// current states of drawing and you can change them by
// calling methods.
CrCanvas canvas(surface);

// Clear surface (optional)
canvas.setSource(1.0, 1.0, 1.0);
canvas.drawPaint();

// Draw something by canvas...

// Save it
surface->flush();
surface->writeToPNG("output.png");
```

You can also make a surface by Drawable:
```c++
// Create a window before calling this.
auto surface = CrSurface::MakeFromDrawable(&window);
```
The "real backend" Cairo2d uses is undefined and depends on your display
server. Cairo2d just converts all the drawing operations to trapezoids
and composition operations, and then sends them to the display server.
The rasterization of trapezoids and image composition is the display server's
responsibility.

For X11 platform, Xorg server may use GLAMOR acceleration when it's available.
In that case, Cairo2d will draw with OpenGL acceleration.

Note that Cairo2d itself also has the ability of rasterization and composition.
The  internal rasterizer and compositor will be used when your display server don't
have the ability of rasterization and composition, which may cause performance
loss.

Cairo2d is a simple 2D engine designed to handle UI rendering, it communicates
with the display server directly. For better performance (with GPU acceleration)
and advanced effects (like fragment shaders, drawing vertices), use Skia2d.

### Skia2d
The __Skia2d__ interface can render content through a CPU rasterizer or a GPU
acclerated pipeline. It's the most important part of Ciallo. All of the drawing
operations from Javascript will be performed by it. The source code is in
`src/Ciallo/Skia2d`.


