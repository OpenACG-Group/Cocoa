#ifndef COCOA_TYPETRAITS_H
#define COCOA_TYPETRAITS_H

#include <type_traits>
#include <vector>

#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

template<typename E, typename = typename std::enable_if<std::is_enum_v<E>>::type>
class Bitfield
{
public:
    using T = typename std::underlying_type<E>::type;
    Bitfield() : fValue(0) {}
    explicit Bitfield(E value) : fValue(static_cast<T>(value)) {}
    explicit Bitfield(const std::vector<E>& values) : fValue(0) {
        for (E v : values)
            fValue |= static_cast<T>(v);
    }
    explicit Bitfield(T dv) : fValue(dv) {}

    Bitfield(const std::initializer_list<E>& values) : fValue(0) {
        for (E v : values)
            fValue |= static_cast<T>(v);
    }

    inline Bitfield& operator|=(const E bit) {
        fValue |= static_cast<T>(bit);
        return *this;
    }

    va_nodiscard inline Bitfield operator|(const E bit) {
        Bitfield result = *this;
        result.fValue |= static_cast<T>(bit);
        return result;
    }

    va_nodiscard inline bool operator&(const E bit) {
        return (fValue & static_cast<T>(bit)) == static_cast<T>(bit);
    }

    va_nodiscard inline T value() {
        return fValue;
    }

private:
    T   fValue;
};

VANILLA_NS_END
#endif //COCOA_TYPETRAITS_H
