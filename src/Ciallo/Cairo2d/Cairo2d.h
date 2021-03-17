#ifndef COCOA_CAIRO2D_H
#define COCOA_CAIRO2D_H

#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

using CrScalar = double;
class CrRect
{
public:
    CrRect(double x, double y, double width, double height)
            : fRect{ .x = x, .y = y, .width = width, .height = height } {}

    inline double x() const        { return fRect.x; }
    inline double y() const        { return fRect.y; }
    inline double width() const    { return fRect.width; }
    inline double height() const   { return fRect.height; }

    inline cairo_rectangle_t *ptr()                   { return &fRect; }
    inline const cairo_rectangle_t *const_ptr() const { return &fRect; }

private:
    cairo_rectangle_t fRect;
};

CIALLO_END_NS
#endif //COCOA_CAIRO2D_H
