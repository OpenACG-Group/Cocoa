#ifndef COCOA_UNIQUEPERSISTENT_H
#define COCOA_UNIQUEPERSISTENT_H

#include <utility>
#include <stdexcept>

namespace cocoa {

template<typename T>
class UniquePersistent
{
public:
    static T *Instance() {
        if (fpSelf == nullptr)
            throw std::runtime_error("No available instance");
        return fpSelf;
    }

    static T& Ref() {
        return *Instance();
    }

    template<typename...ArgsT>
    static void New(ArgsT&&...args) {
        fpSelf = new T(std::forward<ArgsT>(args)...);
    }

    static void Delete() {
        delete fpSelf;
    }

private:
    static T *fpSelf;
};

template<typename T>
T *UniquePersistent<T>::fpSelf = nullptr;

}

#endif //COCOA_UNIQUEPERSISTENT_H
