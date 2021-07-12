#include "Vanilla/Base.h"
VANILLA_NS_BEGIN

VaVec2f operator+(const VaVec2f& a, const VaVec2f& b)
{
    return VaVec2f(a.x() + b.x(), a.y() + b.y());
}

VaVec2f operator-(const VaVec2f& a)
{
    return VaVec2f(-a.x(), -a.y());
}

VaVec2f operator-(const VaVec2f& a, const VaVec2f& b)
{
    return VaVec2f(a.x() - b.x(), a.y() - b.y());
}

VaVec2f operator*(const VaVec2f& a, VaScalar scalar)
{
    return VaVec2f(a.x() * scalar, a.y() * scalar);
}

std::ostream& operator<<(std::ostream& os, const VaVec2f& v)
{
    os << '(' << v.x() << ", " << v.y() << ')';
    return os;
}

VANILLA_NS_END
