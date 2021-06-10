Vanilla 2D Rendering Engine
===========================

Introduction
------------
__Vanilla__ is a fast, high-performance and hardware-accelarated 2D rendering engine which
has been created for Cocoa Project. This backend is also a standalone 2D rendering engine
that can be used in other projects which requires 2D rendering ability.

Vanilla consists of an window system abstraction layer and a backend layer. The __Window System
Abstraction (WSA)__ provides standard APIs so that you can interact with different window systems
by a standard interface.

Signal and Slots
----------------
It's really complicated to handle each events which is sent from window system. For an example,
a window may recieve these events:
* Map: A window is displayed
