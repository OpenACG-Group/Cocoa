#ifndef COCOA_RANDOMDEVICE_H
#define COCOA_RANDOMDEVICE_H

#include <cstdint>

namespace cocoa {

class RandomDevice
{
public:
    enum class Device
    {
        kLinuxDevRandom,
        kLinuxDevURandom
    };

    explicit RandomDevice(Device type);
    ~RandomDevice();

    template<typename T>
    T next() {
        T t;
        this->readData(&t, sizeof(T));
        return t;
    }

    template<>
    double next() {
        auto origin = this->next<uint64_t>() & 1000000UL;
        return double(origin) / 1e6;
    }

private:
    void readData(void *ptr, size_t size) const;

    int     fDeviceFd;
};

} // namespace cocoa
#endif //COCOA_RANDOMDEVICE_H
