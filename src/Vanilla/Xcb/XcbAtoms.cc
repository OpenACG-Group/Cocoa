#include "Core/Errors.h"

#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/Xcb/XcbAtoms.h"
VANILLA_NS_BEGIN

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Vanilla)

namespace {

xcb_atom_t intern_atom(xcb_connection_t *connection, const std::string& name, bool onlyIfExist)
{
    auto cookie = xcb_intern_atom(connection, onlyIfExist, name.size(), name.c_str());
    xcb_generic_error_t *err = nullptr;
    UniqueHandle<xcb_intern_atom_reply_t> reply(xcb_intern_atom_reply(connection, cookie, &err));
    if (err != nullptr)
    {
        QLOG(LOG_ERROR, "Failed to intern X11 atom {}, error code = XCB:{}", name, err->error_code);
        throw VanillaException(__func__, "Failed to intern X11 atom");
    }

    return reply->atom;
}

} // namespace anonymous

XcbAtoms::XcbAtoms(xcb_connection_t *conn)
    : fConnection(conn),
      fAtoms{}
{
    fAtoms[AtomType::WM_DELETE_WINDOW] = intern_atom(fConnection, "WM_DELETE_WINDOW", false);
    fAtoms[AtomType::_NET_WM_ICON] = intern_atom(fConnection, "_NET_WM_ICON", false);
    fAtoms[AtomType::CARDINAL] = intern_atom(fConnection, "CARDINAL", false);
    fAtoms[AtomType::_VA_CLOSE_CONNECTION] = intern_atom(fConnection, "_VA_CLOSE_CONNECTION", false);
    fAtoms[AtomType::WM_PROTOCOLS] = intern_atom(fConnection, "WM_PROTOCOLS", false);
}

xcb_atom_t XcbAtoms::get(AtomType type)
{
    CHECK(type < AtomType::LAST_ATOM);
    return fAtoms[type];
}

VANILLA_NS_END
