#ifndef COCOA_MATH_H
#define COCOA_MATH_H

#include <cstdint>

#include "NaGui/Base.h"
NAGUI_NS_BEGIN

template<typename T>
struct Vec2
{
    Vec2(T&& v1, T&& v2)
        : s0(v1), s1(v2) {}

    Vec2<T>& operator=(const Vec2<T>& v)
    {
        s0 = v.s0;
        s1 = v.s1;
        return *this;
    }
    T   s0;
    T   s1;
};

template<typename T>
struct Vec3
{
    Vec3(T&& v1, T&& v2, T&& v3)
        : s0(v1), s1(v2), s2(v3) {}

    Vec3<T>& operator=(const Vec3<T>& v)
    {
        s0 = v.s0;
        s1 = v.s1;
        s2 = v.s2;
        return *this;
    }
    T   s0;
    T   s1;
    T   s2;
};

template<typename T>
Vec2<T> operator+(Vec2<T>& v1, Vec2<T>& v2)
{
    return Vec2<T>(v1.s0 + v2.s0, v1.s1 + v2.s1);
}

template<typename T>
bool operator==(const Vec2<T>& v1, const Vec2<T>& v2)
{
    return v1.s0 == v2.s0 && v1.s1 == v2.s1;
}

template<typename T>
Vec3<T> operator+(Vec3<T>& v1, Vec3<T>& v2)
{
    return Vec3<T>(v1.s0 + v2.s0,
                   v1.s1 + v2.s1,
                   v1.s2 + v2.s2);
}

template<typename T>
bool operator==(const Vec3<T>& v1, const Vec3<T>& v2)
{
    return v1.s0 == v2.s0 &&
           v1.s1 == v2.s1 &&
           v1.s2 == v2.s2;
}

using Vec2i = Vec2<int32_t>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;

using Vec3i = Vec3<int32_t>;
using Vec3f = Vec3<float>;
using Vec3d = Vec3<double>;

NAGUI_NS_END
#endif //COCOA_MATH_H
