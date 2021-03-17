#include <vector>

#include "Ciallo/XcbAtom.h"
CIALLO_BEGIN_NS

namespace {

struct AtomStringMap
{
    XcbAtom::Atom atom;
    const char *pStr;
};

const AtomStringMap atom_str[] = {
        { XcbAtom::Atom::WM_PROTOCOLS, "WM_PROTOCOLS" },
        { XcbAtom::Atom::WM_DELETE_WINDOW, "WM_DELETE_WINDOW" },
        { XcbAtom::Atom::WM_TAKE_FOCUS, "WM_TAKE_FOCUS" },
        { XcbAtom::Atom::CA_CLOSE_CONNECTION, "CA_CLOSE_CONNECTION" }
};

}  // namespace anonymous

void XcbAtom::initialize(xcb_connection_t *connection)
{
    std::vector<xcb_intern_atom_cookie_t> cookies;
    for (const AtomStringMap& a : atom_str)
    {
        cookies.push_back(xcb_intern_atom(connection,
                                          false,
                                          std::strlen(a.pStr),
                                          a.pStr));
    }

    int32_t i = 0;
    for (xcb_intern_atom_cookie_t c : cookies)
    {
        auto *reply = xcb_intern_atom_reply(connection, c, nullptr);
        BeforeLeaveScope before([&]() -> void {
            ++i;
            if (reply)
                std::free(reply);
        });

        if (!reply)
            continue;
        fAtoms[atom_str[i].atom] = reply->atom;
    }
}

xcb_atom_t XcbAtom::atom(Atom atom)
{
    if (!fAtoms.contains(atom))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("No such X11 atom")
                .make<RuntimeException>();
    }
    return fAtoms[atom];
}

CIALLO_END_NS
