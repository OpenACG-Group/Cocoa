#ifndef COCOA_XCBATOM_H
#define COCOA_XCBATOM_H

#include <map>

#include <xcb/xcb.h>

#include "Ciallo/Ciallo.h"
CIALLO_BEGIN_NS

class XcbAtom
{
public:
    enum class Atom
    {
        /* Window manager/client protocols */
        WM_PROTOCOLS = 0,
        WM_DELETE_WINDOW,
        WM_TAKE_FOCUS,

        /* Cocoa/Ciallo specific */
        CA_CLOSE_CONNECTION,

        LAST_ATOM
    };

    void initialize(xcb_connection_t *connection);
    xcb_atom_t atom(Atom atom);

private:
    std::map<Atom, xcb_atom_t> fAtoms;
};

CIALLO_END_NS
#endif //COCOA_XCBATOM_H
