#ifndef COCOA_VAX11ATOMS_H
#define COCOA_VAX11ATOMS_H

#include <X11/Xlib.h>
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

class VaX11Atoms
{
public:
    enum AtomType
    {
        WM_DELETE_WINDOW = 0,
        _NET_WM_ICON = 1,
        CARDINAL = 2,
        LAST_ATOM
    };

    explicit VaX11Atoms(Display *display);
    ~VaX11Atoms() = default;

    Atom& get(AtomType type);

private:
    Display         *fDisplay;
    Atom             fAtoms[AtomType::LAST_ATOM];
};

VANILLA_NS_END
#endif //COCOA_VAX11ATOMS_H
