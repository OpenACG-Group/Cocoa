#ifndef COCOA_VADBUSCONNECTION_H
#define COCOA_VADBUSCONNECTION_H

#include "dbus-cxx-2.0/dbus-cxx.h"

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

#define VA_DBUS_SERVICE_NAME    "org.OpenACG.Cocoa"

struct VaDBusConnectionPrivate;
class VaDBusConnection
{
public:
    enum BusType
    {
        kSession,
        kSystem
    };

    explicit VaDBusConnection(VaDBusConnectionPrivate *pBus);
    ~VaDBusConnection();

    va_nodiscard Handle<DBus::Connection> get();

    static Handle<VaDBusConnection> Connect(BusType type);

    va_nodiscard inline VaDBusConnectionPrivate *priv()
    { return fPrivate; }

private:
    VaDBusConnectionPrivate       *fPrivate;
};

VANILLA_NS_END
#endif //COCOA_VADBUSCONNECTION_H
