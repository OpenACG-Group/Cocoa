#include <fcntl.h>
#include <unistd.h>

#include "Core/RandomDevice.h"
#include "Core/Exception.h"
namespace cocoa {

RandomDevice::RandomDevice(Device type)
    : fDeviceFd(-1)
{
    char const *path = nullptr;
    switch (type)
    {
    case Device::kLinuxDevRandom:
        path = "/dev/random";
        break;

    case Device::kLinuxDevURandom:
        path = "/dev/urandom";
        break;
    }

    fDeviceFd = open(path, O_RDONLY);
    if (fDeviceFd < 0)
        throw RuntimeException(__func__, "Failed to open system random device");
}

RandomDevice::~RandomDevice()
{
    if (fDeviceFd >= 0)
        close(fDeviceFd);
}

void RandomDevice::readData(void *ptr, size_t size) const
{
    if (read(fDeviceFd, ptr, size) != size)
        throw RuntimeException(__func__, "Failed to read system random device");
}

} // namespace cocoa