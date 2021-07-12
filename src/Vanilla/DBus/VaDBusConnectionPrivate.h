#ifndef COCOA_VADBUSCONNECTIONPRIVATE_H
#define COCOA_VADBUSCONNECTIONPRIVATE_H

#include "dbus-cxx-2.0/dbus-cxx.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

struct VaDBusConnectionPrivate
{
    Handle<DBus::Dispatcher> dispatcher;
    Handle<DBus::Connection> connection;
};

VANILLA_NS_END
#endif //COCOA_VADBUSCONNECTIONPRIVATE_H
