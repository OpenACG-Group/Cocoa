#include <stdio.h>
#include <limits.h>
#include <X11/Xlib.h>

Bool PrintSelection(Display *display, Window window, const char *bufname, const char *fmtname)
{
    char *result;
    unsigned long ressize, restail;
    int resbits;
    Atom bufid = XInternAtom(display, bufname, False),
            fmtid = XInternAtom(display, fmtname, False),
            propid = XInternAtom(display, "XSEL_DATA", False),
            incrid = XInternAtom(display, "INCR", False);
    XEvent event;

    XConvertSelection(display, bufid, fmtid, propid, window, CurrentTime);
    do {
        XNextEvent(display, &event);
    } while (event.type != SelectionNotify || event.xselection.selection != bufid);

    if (event.xselection.property)
    {
        XGetWindowProperty(display, window, propid, 0, LONG_MAX/4, False, AnyPropertyType,
                           &fmtid, &resbits, &ressize, &restail, (unsigned char**)&result);

        if (fmtid == incrid)
            printf("Buffer is too large and INCR reading is not implemented yet.\n");
        else
            printf("%.*s", (int)ressize, result);

        XFree(result);
        return True;
    }
    else // request failed, e.g. owner can't convert to the target format
        return False;
}

int main()
{
    Display *display = XOpenDisplay(NULL);
    unsigned long color = BlackPixel(display, DefaultScreen(display));
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0,0, 1,1, 0, color, color);
    Bool result = PrintSelection(display, window, "CLIPBOARD", "UTF8_STRING") ||
                  PrintSelection(display, window, "CLIPBOARD", "STRING");
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return !result;
}
