#ifndef COCOA_CORE_UNIQUEPERSISTENT_H
#define COCOA_CORE_UNIQUEPERSISTENT_H

#include <utility>
#include <stdexcept>

namespace cocoa {

template<typename T>
class UniquePersistent
{
public:
    static T *Instance() {
        if (self_pointer_ == nullptr)
            throw std::runtime_error("No available instance");
        return self_pointer_;
    }

    static T& Ref() {
        return *Instance();
    }

    template<typename...ArgsT>
    static void New(ArgsT&&...args) {
        self_pointer_ = new T(std::forward<ArgsT>(args)...);
    }

    static void Delete() {
        delete self_pointer_;
    }

private:
    static T *self_pointer_;
};

template<typename T>
T *UniquePersistent<T>::self_pointer_ = nullptr;

}

#endif //COCOA_CORE_UNIQUEPERSISTENT_H
