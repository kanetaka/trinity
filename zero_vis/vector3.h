#pragma once

#include "math.h"
#include "matrix.h"

template <typename T> struct Matrix3;
template <typename T> struct Matrix4;
template <typename T> struct Quaternion;

template <typename T>
struct Vector3
{
    union {
        struct { T x_; T y_; T z_; };
        struct { T r_; T g_; T b_; };
        T data_[3];
    };

    inline Vector3() :
        x_(0.0),
        y_(0.0),
        z_(0.0)
    {
    }

    inline explicit Vector3(T x, T y, T z) :
        x_(x),
        y_(y),
        z_(z)
    {
    }

    inline void Set(T x, T y, T z)
    {
        x_ = x;
        y_ = y;
        z_ = z;
    }

    inline T LengthSq() const
    {
        return (x_*x_ + y_*y_ + z_*z_);
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
        z_ /= length;
    }

    inline static Vector3 Normalize(const Vector3& vec)
    {
        Vector3 tmp = vec;
        tmp.Normalize();
        return tmp;
    }

    inline static T Dot(const Vector3& a, const Vector3& b)
    {
        return (a.x_ * b.x_ + a.y_ * b.y_ + a.z_ * b.z_);
    }

    inline static Vector3 Cross(const Vector3& a, const Vector3& b)
    {
        Vector3 tmp;
        tmp.x_ = a.y_ * b.z_ - a.z_ * b.y_;
        tmp.y_ = a.z_ * b.x_ - a.x_ * b.z_;
        tmp.z_ = a.x_ * b.y_ - a.y_ * b.x_;
        return tmp;
    }

    inline static Vector3 Lerp(const Vector3& a, const Vector3& b, T f)
    {
        return Vector3(a + f * (b - a));
    }

    inline static Vector3 Reflect(const Vector3& v, const Vector3& n)
    {
        return v - 2.0 * Vector3::Dot(v, n) * n;
    }

    inline static Vector3 Transform(const Matrix4<T>& mat, const Vector3& vec, T w = 1.0)
    {
        Vector3 res;
        res.x_ = mat.m00_ * vec.x_ + mat.m01_ * vec.y_ + mat.m02_ * vec.z_ + mat.m03_ * w;
        res.y_ = mat.m10_ * vec.x_ + mat.m11_ * vec.y_ + mat.m12_ * vec.z_ + mat.m13_ * w;
        res.z_ = mat.m20_ * vec.x_ + mat.m21_ * vec.y_ + mat.m22_ * vec.z_ + mat.m23_ * w;
        return res;
    }

    inline static Vector3 TransformWithPerspDiv(const Matrix4<T>& mat, const Vector3& vec, T w = 1.0)
    {
        Vector3 res;
        res.x_ = mat.m00_ * vec.x_ + mat.m01_ * vec.y_ + mat.m02_ * vec.z_ + mat.m03_ * w;
        res.y_ = mat.m10_ * vec.x_ + mat.m11_ * vec.y_ + mat.m12_ * vec.z_ + mat.m13_ * w;
        res.z_ = mat.m20_ * vec.x_ + mat.m21_ * vec.y_ + mat.m22_ * vec.z_ + mat.m23_ * w;
        T transformedW = mat.m30_ * vec.x_ + mat.m31_ * vec.y_ + mat.m32_ * vec.z_ + mat.m33_ * w;

        if (!Math::IsNearZero(Math::Abs(transformedW))) {
            transformedW = 1.0 / transformedW;
            res *= transformedW;
        }
        return res;
    }

    inline static Vector3 Transform(const Quaternion<T>& q, const Vector3& v)
    {
        Vector3 qv(q.x_, q.y_, q.z_);
        Vector3 res = v;
        res += 2.0 * Vector3::Cross(qv, Vector3::Cross(qv, v) + q.w_ * v);
        return res;
    }

    inline Vector3& operator=(const Vector3& rhs)
    {
        x_ = rhs.x_;
        y_ = rhs.y_;
        z_ = rhs.z_;
        return *this;
    }

    inline Vector3& operator+=(const Vector3& rhs)
    {
        x_ += rhs.x_;
        y_ += rhs.y_;
        z_ += rhs.z_;
        return *this;
    }

    inline Vector3& operator-=(const Vector3& rhs)
    {
        x_ -= rhs.x_;
        y_ -= rhs.y_;
        z_ -= rhs.z_;
        return *this;
    }

    inline Vector3& operator*=(const Vector3& rhs)
    {
        x_ *= rhs.x_;
        y_ *= rhs.y_;
        z_ *= rhs.z_;
        return *this;
    }

    inline Vector3& operator*=(T rhs)
    {
        x_ *= rhs;
        y_ *= rhs;
        z_ *= rhs;
        return *this;
    }

    inline Vector3& operator/=(T rhs)
    {
        x_ /= rhs;
        y_ /= rhs;
        z_ /= rhs;
        return *this;
    }

    inline Vector3 operator+() const
    {
        return Vector3(x_, y_, z_);
    }

    inline Vector3 operator-() const
    {
        return Vector3(-x_, -y_, -z_);
    }

    inline friend Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
    {
        return Vector3(lhs.x_ + rhs.x_, lhs.y_ + rhs.y_, lhs.z_ + rhs.z_);
    }

    inline friend Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
    {
        return Vector3(lhs.x_ - rhs.x_, lhs.y_ - rhs.y_, lhs.z_ - rhs.z_);
    }

    // Component-wise Multply
    inline friend Vector3 operator*(const Vector3& lhs, const Vector3& rhs)
    {
        return Vector3(lhs.x_ * rhs.x_, lhs.y_ * rhs.y_, lhs.z_ * rhs.z_);
    }

    inline friend Vector3 operator*(const Vector3& lhs, T rhs)
    {
        return Vector3(lhs.x_ * rhs, lhs.y_ * rhs, lhs.z_ * rhs);
    }

    inline friend Vector3 operator*(T lhs, const Vector3& rhs)
    {
        return Vector3(rhs.x_ * lhs, rhs.y_ * lhs, rhs.z_ * lhs);
    }

    inline friend Vector3 operator*(const Matrix3<T>& lhs, const Vector3& rhs)
    {
        Vector3 res;
        res.x = lhs.m00_ * rhs.x_ + lhs.m01_ * rhs.y_ + lhs.m02_ * rhs.z_;
        res.y = lhs.m10_ * rhs.x_ + lhs.m11_ * rhs.y_ + lhs.m12_ * rhs.z_;
        res.z = lhs.m20_ * rhs.x_ + lhs.m21_ * rhs.y_ + lhs.m22_ * rhs.z_;
        return res;
    }

    inline friend Vector3 operator/(const Vector3& lhs, T rhs)
    {
        return Vector3(lhs.x_ / rhs, lhs.y_ / rhs, lhs.z_ / rhs);
    }

    static const Vector3 ZERO;
    static const Vector3 UNIT_X;
    static const Vector3 UNIT_Y;
    static const Vector3 UNIT_Z;
    static const Vector3 NEG_UNIT_X;
    static const Vector3 NEG_UNIT_Y;
    static const Vector3 NEG_UNIT_Z;
    static const Vector3 INFINITY;
    static const Vector3 NEG_INFINITY;
};

template<typename T> const Vector3<T> Vector3<T>::ZERO(0.0, 0.0, 0.0);
template<typename T> const Vector3<T> Vector3<T>::UNIT_X(1.0, 0.0, 0.0);
template<typename T> const Vector3<T> Vector3<T>::UNIT_Y(0.0, 1.0, 0.0);
template<typename T> const Vector3<T> Vector3<T>::UNIT_Z(0.0, 0.0, 1.0);
template<typename T> const Vector3<T> Vector3<T>::NEG_UNIT_X(-1.0, 0.0, 0.0);
template<typename T> const Vector3<T> Vector3<T>::NEG_UNIT_Y(0.0, -1.0, 0.0);
template<typename T> const Vector3<T> Vector3<T>::NEG_UNIT_Z(0.0, 0.0, -1.0);
template<typename T> const Vector3<T> Vector3<T>::INFINITY(Math::INFINITY<T>, Math::INFINITY<T>, Math::INFINITY<T>);
template<typename T> const Vector3<T> Vector3<T>::NEG_INFINITY(Math::NEG_INFINITY<T>, Math::NEG_INFINITY<T>, Math::NEG_INFINITY<T>);

template <typename T>
using Vec3 = Vector3<T>;
using Vec3f = Vector3<float>;
using Vec3d = Vector3<double>;
using Rgb = Vector3<float>;
