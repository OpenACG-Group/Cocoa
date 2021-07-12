#include <iostream>
#include <type_traits>

template<typename E, typename = typename std::enable_if<std::is_enum_v<E>>::type>
class Bitfield
{
public:
    using T = typename std::underlying_type<E>::type;
    Bitfield() : fValue(0) {}
    explicit Bitfield(E value) : fValue(static_cast<T>(value)) {}

    Bitfield& operator|=(const E bit)
    {
        fValue |= static_cast<T>(bit);
        return *this;
    }

    Bitfield operator|(const E bit)
    {
        Bitfield result = *this;
        result.fValue |= static_cast<T>(bit);
        return result;
    }

    bool operator&(const E bit)
    {
        return (fValue & static_cast<T>(bit)) == static_cast<T>(bit);
    }

private:
    T   fValue;
};

enum class Element : uint32_t
{
    A = (1 << 0),
    B = (1 << 1),
    C = (1 << 2)
};

int main(int argc, const char **argv)
{
    Bitfield<Element> field;
    field |= Element::A;
    field |= Element::B;

    std::cout << "test A: " << (field & Element::A) << std::endl;
    std::cout << "test B: " << (field & Element::B) << std::endl;
    std::cout << "test C: " << (field & Element::C) << std::endl;

    return 0;
}
