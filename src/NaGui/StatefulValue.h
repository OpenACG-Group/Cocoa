#ifndef COCOA_STATEFULVALUE_H
#define COCOA_STATEFULVALUE_H

#include <atomic>

#include "NaGui/NaGui.h"
NAGUI_NS_BEGIN

template<typename T>
class StatefulValueT
{
public:
    template<typename...ArgsT>
    explicit StatefulValueT(ArgsT&&... args)
        : fValue(std::forward<ArgsT>(args)...),
          fChanged(true) {}

    ~StatefulValueT() = default;

    /* StatefulValueT is uncopyable and unmovable */
    StatefulValueT(const StatefulValueT<T>&) = delete;
    StatefulValueT<T>& operator=(const StatefulValueT<T>&) = delete;
    StatefulValueT(StatefulValueT<T>&&) = delete;
    StatefulValueT<T>& operator=(StatefulValueT<T>&&) = delete;

    void set(const T& value)
    {
        if (value != fValue)
        {
            fChanged.store(true);
            fValue = value;
        }
    }

    T& value()
    {
        return fValue;
    }

    bool changed()
    {
        if (fChanged.load())
        {
            fChanged.store(false);
            return true;
        }
        return false;
    }

private:
    T                   fValue;
    std::atomic_bool    fChanged;
};

NAGUI_NS_END
#endif //COCOA_STATEFULVALUE_H
