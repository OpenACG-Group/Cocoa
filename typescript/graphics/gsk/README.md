# [TSLib:graphics/Gsk] Graph Scene Kit

The Gsk library contains an implementation of 2D scenegraph system for Cocoa,
based on the native module `renderer`.

## Basic Concepts
A __scenegraph__ represents the attributes of geometric objects used for drawing
and their interrelationships (e.g., hierarchy, clipping, etc.). 
Gsk creates a single class for every node in the scenegraph.