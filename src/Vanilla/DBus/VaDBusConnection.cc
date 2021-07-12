#include "Core/Journal.h"
#include "Vanilla/Base.h"
#include "Vanilla/DBus/VaDBusConnection.h"
#include "Vanilla/DBus/VaDBusConnectionPrivate.h"
VANILLA_NS_BEGIN

Handle<VaDBusConnection> VaDBusConnection::Connect(BusType type)
{
    auto priv = new VaDBusConnectionPrivate;

    priv->dispatcher = DBus::StandaloneDispatcher::create();
    if (!priv->dispatcher)
    {
        delete priv;
        throw VanillaException(__func__, "Failed to create a dispatcher for DBus");
    }

    switch (type)
    {
    case BusType::kSession:
        priv->connection = priv->dispatcher->create_connection(DBus::BusType::SESSION);
        break;
    case BusType::kSystem:
        priv->connection = priv->dispatcher->create_connection(DBus::BusType::SYSTEM);
        break;
    }

    if (!priv->connection)
    {
        delete priv;
        throw VanillaException(__func__, "Failed to connect to DBus server");
    }

    DBus::RequestNameResponse response = priv->connection->request_name(VA_DBUS_SERVICE_NAME,
                                                                        DBUSCXX_NAME_FLAG_REPLACE_EXISTING);
    if (response != DBus::RequestNameResponse::PrimaryOwner)
    {
        priv->connection = nullptr;
        delete priv;
        throw VanillaException(__func__, "Failed to request a service name");
    }

    char const *serverId = "<Unknown>";
    if (priv->connection->server_id() && std::strlen(priv->connection->server_id()))
        serverId = priv->connection->server_id();

    Journal::Ref()(LOG_INFO, R"(Connected to DBus server, details:
  %fg<hl>Service name:%reset {}
  %fg<hl>Server ID:%reset %fg<gr>{}%reset)", VA_DBUS_SERVICE_NAME, serverId);

    return std::make_shared<VaDBusConnection>(priv);
}

VaDBusConnection::VaDBusConnection(VaDBusConnectionPrivate *pBus)
    : fPrivate(pBus)
{
}

VaDBusConnection::~VaDBusConnection()
{
    delete fPrivate;
}

Handle<DBus::Connection> VaDBusConnection::get()
{
    return fPrivate->connection;
}

VANILLA_NS_END
