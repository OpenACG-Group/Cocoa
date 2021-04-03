import cairo as Cairo
import math


def draw_arc(ctx, x, y, w, h):
    ctx.set_source_rgb(1, 0, 0)
    ctx.set_line_width(2)
    ctx.rectangle(x, y, w, h)
    ctx.stroke()

    ox, oy = x + w / 2, y + h / 2
    saved_matrix = ctx.get_matrix()
    ctx.translate(ox, oy)
    ctx.scale(w / h, 1)
    ctx.translate(-ox, -oy)
    ctx.arc(ox, oy, h / 2, 0, math.pi * 2)
    ctx.set_matrix(saved_matrix)

    ctx.set_source_rgb(0, 0, 1)
    ctx.stroke()

surface = Cairo.ImageSurface(Cairo.FORMAT_ARGB32, 800, 600)
ctx = Cairo.Context(surface)
ctx.set_source_rgb(0x66 / 0xff, 0xcc / 0xff, 1)
ctx.paint()
draw_arc(ctx, 200, 200, 200, 100)
surface.write_to_png("cairo-draw-arc.png")
