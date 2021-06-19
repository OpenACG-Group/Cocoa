#ifndef COCOA_VAXCBATOMS_H
#define COCOA_VAXCBATOMS_H

#include <xcb/xcb.h>
#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

#define RESERVED_NAME(s) _ ##s

class VaXcbAtoms
{
public:
    enum AtomType
    {
        WM_DELETE_WINDOW = 0,
        WM_PROTOCOLS,
        RESERVED_NAME(NET_WM_ICON),
        CARDINAL,
        RESERVED_NAME(VA_CLOSE_CONNECTION),
        LAST_ATOM
    };

    explicit VaXcbAtoms(xcb_connection_t *display);
    ~VaXcbAtoms() = default;

    xcb_atom_t get(AtomType type);

private:
    xcb_connection_t *fConnection;
    xcb_atom_t        fAtoms[AtomType::LAST_ATOM];
};
#undef RESERVED_NAME

VANILLA_NS_END

#endif //COCOA_VAXCBATOMS_H
