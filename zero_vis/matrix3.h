#pragma once

#include "math.h"
#include "vector.h"
#include "quaternion.h"

template <typename T> struct Vector2;
template <typename T> struct Vector3;
template <typename T> struct Quaternion;

// 3x3 Matrix
template<typename T>
struct Matrix3
{
    union {
        struct {
            T m00_, m01_, m02_;
            T m10_, m11_, m12_;
            T m20_, m21_, m22_;
        };
        T data_[9];
    };

    inline Matrix3()
    {
        *this = Matrix3::IDENTITY;
    }

    inline Matrix3(
            T m00, T m01, T m02,
            T m10, T m11, T m12,
            T m20, T m21, T m22) :
            m00_(m00), m01_(m01), m02_(m02),
            m10_(m10), m11_(m11), m12_(m12),
            m20_(m20), m21_(m21), m22_(m22)
    {
    }

    inline explicit Matrix3(T data[9])
    {
        memcpy(data_, data, 9 * sizeof(T));
    }

    inline static Matrix3 GetTranslation(const Vec2<T>& trans)
    {
        return Matrix3(
            0.0, 0.0, trans.x_,
            0.0, 0.0, trans.y_,
            0.0, 0.0, 1.0);
    }

    inline static Matrix3 GetRotation(T theta)
    {
        T cos = Math::Cos(theta);
        T sin = Math::Sin(theta);
        return Matrix3(
            cos, -sin, 0.0,
            sin,  cos, 0.0,
            0.0,  0.0, 1.0);
    }

    inline static Matrix3 GetScale(T sx, T sy)
    {
        Matrix3 res;
        res.m00_ =  sx; res.m01_ = 0.0; res.m02_ = 0.0;
        res.m10_ = 0.0; res.m11_ =  sy; res.m12_ = 0.0;
        res.m20_ = 0.0; res.m21_ = 0.0; res.m22_ = 1.0;
        return res;
    }

    inline static Matrix3 GetScale(const Vec2<T>& vec)
    {
        return Scale(vec.x_, vec.y_);
    }

    inline static Matrix3 GetScale(T scale)
    {
        return Scale(scale, scale);
    }

    // Matrix multiplication
    inline friend Matrix3 operator*(const Matrix3& a, const Matrix3& b)
    {
        Matrix3 res;
        res.m00_ = (a.m00_ * b.m00_) + (a.m01_ * b.m10_) + (a.m02_ * b.m20_);
        res.m01_ = (a.m00_ * b.m01_) + (a.m01_ * b.m11_) + (a.m02_ * b.m21_);
        res.m02_ = (a.m00_ * b.m02_) + (a.m01_ * b.m12_) + (a.m02_ * b.m22_);
        res.m10_ = (a.m10_ * b.m00_) + (a.m11_ * b.m10_) + (a.m12_ * b.m20_);
        res.m11_ = (a.m10_ * b.m01_) + (a.m11_ * b.m11_) + (a.m12_ * b.m21_);
        res.m12_ = (a.m10_ * b.m02_) + (a.m11_ * b.m12_) + (a.m12_ * b.m22_);
        res.m20_ = (a.m20_ * b.m00_) + (a.m21_ * b.m10_) + (a.m22_ * b.m20_);
        res.m21_ = (a.m20_ * b.m01_) + (a.m21_ * b.m11_) + (a.m22_ * b.m21_);
        res.m22_ = (a.m20_ * b.m02_) + (a.m21_ * b.m12_) + (a.m22_ * b.m22_);
        return res;
    }

    inline Matrix3& operator*=(const Matrix3& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

    static const Matrix3 ZERO;
    static const Matrix3 IDENTITY;
};

template<typename T> const Matrix3<T> Matrix3<T>::ZERO =
Matrix3<T>(0.0, 0.0, 0.0,
        0.0, 0.0, 0.0,
        0.0, 0.0, 0.0);

template<typename T> const Matrix3<T> Matrix3<T>::IDENTITY =
Matrix3<T>(1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0);

template <typename T>
using Mat3 = Matrix3<T>;
using Mat3f = Matrix3<float>;
using Mat3d = Matrix3<double>;
