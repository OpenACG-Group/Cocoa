#include "Core/DBusConnection.h"
#include "Core/Exception.h"
#include "Core/Journal.h"
#include "Core/Errors.h"

#define THIS_FILE_MODULE COCOA_MODULE_NAME(DBus)

namespace cocoa
{

void callback(const std::string& id)
{
    QLOG(LOG_INFO, "Inspector: Future connection notification from {}", id);
}

DBusService::DBusService(const Options& options)
{
    fDispatcher = DBus::StandaloneDispatcher::create(true);
    if (!fDispatcher)
        throw RuntimeException(__func__, "Failed to create DBus dispatcher");
    if (options.busType != DBus::BusType::NONE)
    {
        fConnection = fDispatcher->create_connection(options.busType);
    }
    else if (options.address)
    {
        fConnection = fDispatcher->create_connection(*options.address);
    }
    else
    {
        fConnection = fDispatcher->create_connection(DBus::BusType::SESSION);
    }
    if (!fConnection)
        throw RuntimeException(__func__, "Failed to connect to DBus daemon");

    if (fConnection->name_has_owner(CORE_DBUS_SERVICE_NAME))
    {
        QLOG(LOG_ERROR, "Cannot request a service name from DBus daemon. "
                        "Another Cocoa with DBus service on is running?");
        throw RuntimeException(__func__, "Failed to request a DBus service name");
    }
    auto response = fConnection->request_name(CORE_DBUS_SERVICE_NAME,
                                              DBUSCXX_NAME_FLAG_REPLACE_EXISTING);
    CHECK(response != DBus::RequestNameResponse::AlreadyOwner);
    QLOG(LOG_INFO, "Connected to DBus daemon '{}' service %fg<hl>{}%reset", fConnection->server_id(),
         CORE_DBUS_SERVICE_NAME);

    auto obj = fConnection->create_object("/Cocoa/Inspector", DBus::ThreadForCalling::CurrentThread);
    obj->create_method<void(std::string)>("notifyFutureConnection", "org.OpenACG.Cocoa.Inspector", sigc::ptr_fun(callback));
}

DBusService::~DBusService()
{
    fConnection.reset();
    fDispatcher.reset();
}

} // namespace cocoa
