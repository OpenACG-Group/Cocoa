import cairo

surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, 400, 400)
ctx = cairo.Context(surface)

ctx.set_source_rgba(1, 1, 1, 1)
ctx.paint()

pattern = cairo.LinearGradient(0, 0, 300, 180)
pattern.add_color_stop_rgba(0, 1, 0, 0, 1)
pattern.add_color_stop_rgba(0.5, 0, 0, 1, 1)
pattern.add_color_stop_rgba(1, 0, 1, 0, 1)
ctx.set_source(pattern)
ctx.rectangle(0, 0, 300, 180)
ctx.fill()

surface.write_to_png("cairo-gradient-rectangle.png")
