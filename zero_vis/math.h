#pragma once

#include <cmath>
#include <memory.h>
#include <limits>
#include <random>

#ifdef INFINITY
#undef INFINITY
#endif

namespace Math
{
template<typename T> constexpr T PI = static_cast<T>(3.14159265358979323846);
template<typename T> constexpr T TWO_PI = PI<T> * static_cast<T>(2.0);
template<typename T> constexpr T PI_OVER2 = PI<T> / static_cast<T>(2.0);
template<typename T> constexpr T INFINITY = std::numeric_limits<T>::infinity();
template<typename T> constexpr T NEG_INFINITY = -std::numeric_limits<T>::infinity();
template<typename T> constexpr T E = static_cast<T>(2.71828182845904523536);
template<typename T> constexpr T E10 = static_cast<T>(0.434294482);
template<typename T> constexpr T E2 = static_cast<T>(1.442695041);
template<typename T> constexpr T EPSILON = static_cast<T>(0.00001);
template<> constexpr float EPSILON<float> = 0.001f;

template<typename T>
inline bool IsNearZero(T val, T epsilon = EPSILON<T>)
{
    if (fabs(val) <= epsilon) {
        return true;
    }
    else {
        return false;
    }
}

template<typename T>
inline bool IsNearEqual(T a, T b)
{
    if (IsNearZero(a - b)) {
        return true;
    }
    else {
        return false;
    }
}

template<typename T>
inline T ToRadians(T degrees)
{
    return degrees * PI<T> / 180.0f;
}

template<typename T>
inline T ToDegrees(T radians)
{
    return radians * 180.0f / PI<T>;
}

template<typename T>
inline T Max(const T& a, const T& b)
{
    return (a < b ? b : a);
}

template<typename T>
inline T Min(const T& a, const T& b)
{
    return (a < b ? a : b);
}

template<typename T>
inline T Clamp(const T& value, const T& lower, const T& upper)
{
    return Min(upper, Max(lower, value));
}

template<typename T>
inline T Abs(T value)
{
    return fabs(value);
}

template<typename T>
inline T Cos(T angle)
{
    return cos(angle);
}

template<typename T>
inline T Sin(T angle)
{
    return sin(angle);
}

template<typename T>
inline T Tan(T angle)
{
    return tan(angle);
}

template<typename T>
inline T Acos(T value)
{
    return acos(value);
}

template<typename T>
inline T Atan2(T y, T x)
{
    return atan2(y, x);
}

template<typename T>
inline T Cot(T angle)
{
    return 1.0 / Tan(angle);
}

template<typename T>
inline T Lerp(T a, T b, T f)
{
    return a + f * (b - a);
}

template<typename T>
inline T Sqrt(T value)
{
    return sqrt(value);
}

template<typename T>
inline T Fmod(T numer, T denom)
{
    return fmod(numer, denom);
}

template<typename T>
inline T Random()
{
    static std::uniform_real_distribution<T> distribution(0.0f, 1.0f);
    static std::mt19937_64 generator;
    return distribution(generator);
}

template<typename T>
inline T Random(T min, T max)
{
    return min + (max - min) * Random<T>();
}

} // namespace Math
