#include <cassert>

#include "Vanilla/Base.h"
#include "Vanilla/X11/VaX11Atoms.h"
VANILLA_NS_BEGIN

VaX11Atoms::VaX11Atoms(Display *display)
    : fDisplay(display),
      fAtoms{}
{
    fAtoms[AtomType::WM_DELETE_WINDOW] = XInternAtom(fDisplay, "WM_DELETE_WINDOW", False);
    fAtoms[AtomType::_NET_WM_ICON] = XInternAtom(fDisplay, "_NET_WM_ICON", False);
    fAtoms[AtomType::CARDINAL] = XInternAtom(fDisplay, "CARDINAL", False);
    fAtoms[AtomType::VA_CLOSE_CONNECTION] = XInternAtom(fDisplay, "VA_CLOSE_CONNECTION", False);
}

Atom& VaX11Atoms::get(AtomType type)
{
    assert(type < AtomType::LAST_ATOM);
    return fAtoms[type];
}

VANILLA_NS_END
