#pragma once

#include "math.h"

template <typename T> struct Matrix3;

template <typename T>
struct Vector2
{
    union {
        struct { T x_; T y_; };
        T data[2];
    };

    inline Vector2() :
        x_(0.0),
        y_(0.0)
    {
    }

    inline explicit Vector2(T x, T y) :
        x_(x),
        y_(y)
    {
    }

    inline void Set(T x, T y)
    {
        x_ = x;
        y_ = y;
    }

    inline T LengthSq() const
    {
        return (x_*x_ + y_*y_);
    }

    inline T Length() const
    {
        return (Math::Sqrt(LengthSq()));
    }

    inline void Normalize()
    {
        T length = Length();
        x_ /= length;
        y_ /= length;
    }

    inline static Vector2 Normalize(const Vector2& vec)
    {
        Vector2 tmp = vec;
        tmp.Normalize();
        return tmp;
    }

    inline static T Dot(const Vector2& a, const Vector2& b)
    {
        return (a.x_ * b.x_ + a.y_ * b.y_);
    }

    inline static Vector2 Lerp(const Vector2& a, const Vector2& b, T f)
    {
        return Vector2(a + f * (b - a));
    }

    inline static Vector2 Reflect(const Vector2& v, const Vector2& n)
    {
        return v - 2.0 * Vector2::Dot(v, n) * n;
    }

    inline static Vector2 Transform(const Matrix3<T>& mat, const Vector2& vec,  T w = 1.0)
    {
        Vector2 res;
        res.x_ = mat.m00_ * vec.x_ + mat.m01_ * vec.y_ + mat.m02_ * w;
        res.y_ = mat.m10_ * vec.x_ + mat.m11_ * vec.y_ + mat.m12_ * w;
        return res;
    }

    inline friend Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
    {
        return Vector2(lhs.x_ + rhs.x_, lhs.y_ + rhs.y_);
    }

    inline friend Vector2 operator-(const Vector2& lhs, const Vector2& rhs)
    {
        return Vector2(lhs.x_ - rhs.x_, lhs.y_ - rhs.y_);
    }

    // 要素ごとの乗算
    inline friend Vector2 operator*(const Vector2& lhs, const Vector2& rhs)
    {
        return Vector2(lhs.x_ * rhs.x_, lhs.y_ * rhs.y_);
    }

    inline friend Vector2 operator*(const Vector2& lhs, T rhs)
    {
        return Vector2(lhs.x_ * rhs, lhs.y_ * rhs);
    }

    inline friend Vector2 operator*(T lhs, const Vector2& rhs)
    {
        return Vector2(lhs * rhs.x_, lhs * rhs.y_);
    }

    inline Vector2& operator*=(T rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        return *this;
    }

    inline Vector2& operator+=(const Vector2& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

    inline Vector2& operator-=(const Vector2& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        return *this;
    }

    static const Vector2 ZERO;
    static const Vector2 UNIT_X;
    static const Vector2 UNIT_Y;
    static const Vector2 NEG_UNIT_X;
    static const Vector2 NEG_UNIT_Y;
};

template<typename T> const Vector2<T> Vector2<T>::ZERO = Vector2<T>(0, 0);
template<typename T> const Vector2<T> Vector2<T>::UNIT_X = Vector2<T>(1, 0);
template<typename T> const Vector2<T> Vector2<T>::UNIT_Y = Vector2<T>(0, 1);
template<typename T> const Vector2<T> Vector2<T>::NEG_UNIT_X = Vector2<T>(-1, 0);
template<typename T> const Vector2<T> Vector2<T>::NEG_UNIT_Y = Vector2<T>(0, -1);

template <class T>
using Vec2 = Vector2<T>;
using Vec2f = Vec2<float>;
using Vec2d = Vec2<double>;
