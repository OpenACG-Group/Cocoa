#ifndef COCOA_CORE_DBUSCONNECTION_H
#define COCOA_CORE_DBUSCONNECTION_H

#include <optional>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#include "dbus-cxx.h"
#pragma clang diagnostic pop

#include "Core/UniquePersistent.h"

namespace cocoa
{

#define CORE_DBUS_TYPE_SYSTEM   "system"
#define CORE_DBUS_TYPE_SESSION  "session"
#define CORE_DBUS_SERVICE_NAME  "org.OpenACG.Cocoa"

class DBusService : public UniquePersistent<DBusService>
{
public:
    struct Options
    {
        DBus::BusType busType{DBus::BusType::SESSION};
        std::optional<std::string> address;
    };

    explicit DBusService(const Options& options);
    ~DBusService();

private:
    std::shared_ptr<DBus::Dispatcher>   fDispatcher;
    std::shared_ptr<DBus::Connection>   fConnection;
};

} // namespace cocoa
#endif //COCOA_CORE_DBUSCONNECTION_H
