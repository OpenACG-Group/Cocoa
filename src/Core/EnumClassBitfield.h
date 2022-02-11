#ifndef COCOA_ENUMCLASSBITFIELD_H
#define COCOA_ENUMCLASSBITFIELD_H

#include <concepts>
#include <initializer_list>
#include <vector>

namespace cocoa {
template<typename E, typename = typename std::enable_if<std::is_enum_v<E>>::type>
class Bitfield
{
public:
    using T = typename std::underlying_type<E>::type;
    explicit Bitfield(T value) : fValue(value) {}
    Bitfield() : fValue(0) {}
    explicit Bitfield(E value) : fValue(static_cast<T>(value)) {}

    explicit Bitfield(const std::vector<E>& values) : fValue(0) {
        for (E v : values)
            fValue |= static_cast<T>(v);
    }

    Bitfield(const std::initializer_list<E>& values) : fValue(0) {
        for (E v : values)
            fValue |= static_cast<T>(v);
    }

    Bitfield& operator|=(const E bit) {
        fValue |= static_cast<T>(bit);
        return *this;
    }

    Bitfield operator|(const E bit) const {
        Bitfield result = *this;
        result.fValue |= static_cast<T>(bit);
        return result;
    }

    bool operator&(const E bit) const {
        return (fValue & static_cast<T>(bit)) == static_cast<T>(bit);
    }

    Bitfield operator~() const {
        Bitfield result = *this;
        result.fValue = ~result.fValue;
        return result;
    }

    Bitfield& operator|=(const Bitfield<E> bit) {
        fValue |= bit.fValue;
        return *this;
    }

    Bitfield operator|(const Bitfield<E> bit) const {
        Bitfield result = *this;
        result.fValue |= bit.fValue;
        return result;
    }

    Bitfield operator&(const Bitfield<E> bit) const {
        Bitfield result = *this;
        result.fValue = result.fValue & bit.fValue;
        return result;
    }

    [[nodiscard]] bool isEmpty() const {
        return fValue == 0;
    }

private:
    T   fValue;
};

}

#endif //COCOA_ENUMCLASSBITFIELD_H
