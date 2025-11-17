#pragma once

template <typename T>
struct Vector4
{
    union {
        struct { T x_; T y_; T z_; T w_; };
        struct { T r_; T g_; T b_; T a_; };
        T data_[4];
    };

    inline Vector4() :
        x_(0.0),
        y_(0.0),
        z_(0.0),
        w_(0.0)
    {
    }

    inline friend Vector4 operator*(const Matrix4<T>& mat, const Vector4& vec)
    {
        Vector4<T> res;
        res.x = mat.m00_ * vec.x_ + mat.m01_ * vec.y_ + mat.m02_ * vec.z_ + mat.m03_ * vec.w_;
        res.y = mat.m10_ * vec.x_ + mat.m11_ * vec.y_ + mat.m12_ * vec.z_ + mat.m13_ * vec.w_;
        res.z = mat.m20_ * vec.x_ + mat.m21_ * vec.y_ + mat.m22_ * vec.z_ + mat.m23_ * vec.w_;
        res.w = mat.m30_ * vec.x_ + mat.m31_ * vec.y_ + mat.m32_ * vec.z_ + mat.m33_ * vec.w_;
        return res;
    }
};

template <class T>
using Vec4 = Vector4<T>;
using Vec4f = Vector4<float>;
using Vec4d = Vector4<double>;
using Rgba = Vector4<float>;
