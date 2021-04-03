#include <map>

#include "Core/Exception.h"
#include "Virtfs/VfsBackendDriver.h"
VIRTFS_NS_BEGIN

namespace {

std::map<std::string, Handle<VfsBackendDriver>> driverTable;

} // namespace anonymous

void VfsBackendDriver::RegisterDriver(const Handle<VfsBackendDriver>& driver)
{
    if (driverTable.contains(driver->name()))
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Virtfs: Virtual filesystem driver \"")
                .append(driver->name())
                .append("\" has been already registered")
                .make<RuntimeException>();
    }
    driverTable[driver->name()] = driver;
}

void VfsBackendDriver::UnregisterDriver(const std::string& driver)
{
    if (!driverTable.contains(driver))
        return;
    if (driverTable[driver].use_count() != 1)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("Virtfs: Failed to unregister driver \"")
                .append(driver)
                .append("\", device busy")
                .make<RuntimeException>();
    }
    driverTable.erase(driver);
}

Handle<VfsBackendDriver> VfsBackendDriver::GetDriver(const std::string& driver)
{
    if (!driverTable.contains(driver))
        return nullptr;
    return driverTable[driver];
}

VIRTFS_NS_END
