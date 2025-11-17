#pragma once

#include "math.h"

// 4x4 Matrix
template<typename T>
struct Matrix4
{
    union {
        struct {
            T m00_, m01_, m02_, m03_;
            T m10_, m11_, m12_, m13_;
            T m20_, m21_, m22_, m23_;
            T m30_, m31_, m32_, m33_;
        };
        T data_[16];
    };

public:
    inline Matrix4()
    {
        *this = Matrix4::IDENTITY;
    }

    inline Matrix4(
            T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33) :
            m00_(m00), m01_(m01), m02_(m02), m03_(m03),
            m10_(m10), m11_(m11), m12_(m12), m13_(m13),
            m20_(m20), m21_(m21), m22_(m22), m23_(m23),
            m30_(m30), m31_(m31), m32_(m32), m33_(m33)
    {
    }

    inline explicit Matrix4(T data[16])
    {
        memcpy(data_, data, 16 * sizeof(T));
    }

public:
    inline Vector3<T> GetTranslation() const
    {
        return Vector3<T>(m03_, m13_, m23_);
    }

    inline Vector3<T> GetScale() const
    {
        Vector3<T> res;
        res.x_ = Vector3<T>(m00_, m01_, m02_).Length();
        res.y_ = Vector3<T>(m10_, m11_, m12_).Length();
        res.z_ = Vector3<T>(m20_, m21_, m22_).Length();
        return res;
    }

    inline Vector3<T> GetAxisX() const
    {
        return Vector3<T>::Normalize(Vector3<T>(m00_, m10_, m20_));
    }

    inline Vector3<T> GetAxisY() const
    {
        return Vector3<T>::Normalize(Vector3<T>(m01_, m11_, m21_));
    }

    inline Vector3<T> GetAxisZ() const
    {
        return Vector3<T>::Normalize(Vector3<T>(m02_, m12_, m22_));
    }

    inline void Invert()
    {
        T b0 = (m20_ * m31_) - (m21_ * m30_);
        T b1 = (m20_ * m32_) - (m22_ * m30_);
        T b2 = (m23_ * m30_) - (m20_ * m33_);
        T b3 = (m21_ * m32_) - (m22_ * m31_);
        T b4 = (m23_ * m31_) - (m21_ * m33_);
        T b5 = (m22_ * m33_) - (m23_ * m32_);
        T d11 = m11_ * b5 + m12_ * b4 + m13_ * b3;
        T d12 = m10_ * b5 + m12_ * b2 + m13_ * b1;
        T d13 = m10_ * -b4 + m11_ * b2 + m13_ * b0;
        T d14 = m10_ * b3 + m11_ * -b1 + m12_ * b0;
        T det = m00_ * d11 - m01_ * d12 + m02_ * d13 - m03_ * d14;

        if (Math::Abs(det) == 0.0) {
            *this = Matrix4::ZERO;
            return;
        }

        det = 1.0 / det;
        T a0 = (m00_ * m11_) - (m01_ * m10_);
        T a1 = (m00_ * m12_) - (m02_ * m10_);
        T a2 = (m03_ * m10_) - (m00_ * m13_);
        T a3 = (m01_ * m12_) - (m02_ * m11_);
        T a4 = (m03_ * m11_) - (m01_ * m13_);
        T a5 = (m02_ * m13_) - (m03_ * m12_);

        T d21 = m01_ *  b5 + m02_ *  b4 + m03_ * b3;
        T d22 = m00_ *  b5 + m02_ *  b2 + m03_ * b1;
        T d23 = m00_ * -b4 + m01_ *  b2 + m03_ * b0;
        T d24 = m00_ *  b3 + m01_ * -b1 + m02_ * b0;

        T d31 = m31_ *  a5 + m32_ *  a4 + m33_ * a3;
        T d32 = m30_ *  a5 + m32_ *  a2 + m33_ * a1;
        T d33 = m30_ * -a4 + m31_ *  a2 + m33_ * a0;
        T d34 = m30_ *  a3 + m31_ * -a1 + m32_ * a0;

        T d41 = m21_ *  a5 + m22_ *  a4 + m23_ * a3;
        T d42 = m20_ *  a5 + m22_ *  a2 + m23_ * a1;
        T d43 = m20_ * -a4 + m21_ *  a2 + m23_ * a0;
        T d44 = m20_ *  a3 + m21_ * -a1 + m22_ * a0;

        m00_ = +d11 * det; m01_ = -d21 * det; m02_ = +d31 * det; m03_ = -d41 * det;
        m10_ = -d12 * det; m11_ = +d22 * det; m12_ = -d32 * det; m13_ = +d42 * det;
        m20_ = +d13 * det; m21_ = -d23 * det; m22_ = +d33 * det; m23_ = -d43 * det;
        m30_ = -d14 * det; m31_ = +d24 * det; m32_ = -d34 * det; m33_ = +d44 * det;
    }

public:
    inline static Matrix4 CreateTranslation(const Vector3<T>& t)
    {
        return Matrix4(
            1.0, 0.0, 0.0, t.x_,
            0.0, 1.0, 0.0, t.y_,
            0.0, 0.0, 1.0, t.z_,
            0.0, 0.0, 0.0,  1.0);
    }

    inline static Matrix4 CreateScale(T sx, T sy, T sz)
    {
        return Matrix4(
            sx, 0.0, 0.0, 0.0,
           0.0,  sy, 0.0, 0.0,
           0.0, 0.0,  sz, 0.0,
           0.0, 0.0, 0.0, 1.0);
    }

    inline static Matrix4 CreateScale(const Vector3<T>& sv)
    {
        return Scale(sv.x_, sv.y_, sv.z_);
    }

    inline static Matrix4 CreateScale(T s)
    {
        return CreateScale(s, s, s);
    }

    inline static Matrix4 CreateRotationX(T theta)
    {
        T cos = Math::Cos(theta);
        T sin = Math::Sin(theta);
        return Matrix4(
            1.0, 0.0,  0.0, 0.0,
            0.0, cos, -sin, 0.0,
            0.0, sin,  cos, 0.0,
            0.0, 0.0,  0.0, 1.0
        );
    }

    inline static Matrix4 CreateRotationY(T theta)
    {
        T cos = Math::Cos(theta);
        T sin = Math::Sin(theta);
        return Matrix4(
             cos, 0.0, sin, 0.0,
             0.0, 1.0, 0.0, 0.0,
            -sin, 0.0, cos, 0.0,
             0.0, 0.0, 0.0, 1.0
        );
    }

    inline static Matrix4 CreateRotationZ(T theta)
    {
        T cos = Math::Cos(theta);
        T sin = Math::Sin(theta);
        return Matrix4(
            cos, -sin, 0.0, 0.0,
            sin,  cos, 0.0, 0.0,
            0.0,  0.0, 1.0, 0.0,
            0.0,  0.0, 0.0, 1.0
        );
    }

    // Right-hand coordinate and z-up
    inline static Matrix4 CreateLookAt(const Vector3<T>& eye, const Vector3<T>& target, const Vector3<T>& up)
    {
        Vector3<T> axis_z = Vector3<T>::Normalize(eye - target); // Right-hand
        Vector3<T> axis_x = Vector3<T>::Normalize(Vector3<T>::Cross(up, axis_z));
        Vector3<T> axis_y = Vector3<T>::Normalize(Vector3<T>::Cross(axis_z, axis_x));

        Vector3<T> trans;
        trans.x_ = -Vector3<T>::Dot(axis_x, eye);
        trans.y_ = -Vector3<T>::Dot(axis_y, eye);
        trans.z_ = -Vector3<T>::Dot(axis_z, eye);

        return Matrix4(
            axis_x.x_, axis_x.y_, axis_x.z_, trans.x_,
            axis_y.x_, axis_y.y_, axis_y.z_, trans.y_,
            axis_z.x_, axis_z.y_, axis_z.z_, trans.z_,
                 0.0,      0.0,      0.0,     1.0
        );
    }

    inline static Matrix4 CreateOrthoOffCenter(T left, T right, T bottom, T top, T near, T far)
    {
        T x = 2.0 / (right - left);
        T y = 2.0 / (top - bottom);
        T z = 1.0 / (near - far); // Right-hand
        T a = (left + right) / (left - right);
        T b = (bottom + top) / (bottom - top);
        T c = -near * z;

        return Matrix4 (
              x, 0.0, 0.0,   a,
            0.0,   y, 0.0,   b,
            0.0, 0.0,   z,   c,
            0.0, 0.0, 0.0, 1.0);
    }

    // Right-hand coordinate
    inline static Matrix4 CreateOrtho(T width, T height, T near, T far)
    {
        T half_w = width * 0.5;
        T half_h = height * 0.5;
        return CreateOrthoOffCenter(-half_w, half_w, -half_h, half_h, near, far);
    }

    inline static Matrix4 CreatePerspectiveOffCenter(T left, T right, T bottom, T top, T near, T far)
    {
        T x = 2.0 * near / (right - left);
        T y = 2.0 * near / (top - bottom);
        T z = far / (near - far); // Right-hand
        T a = (left + right) / (right - left); // Right-hand
        T b = (top + bottom) / (bottom - top); // Right-hand
        T c = -near * z;

        return Matrix4 (
                x, 0.0,    a, 0.0,
              0.0,   y,    b, 0.0,
              0.0, 0.0,    z,   c,
              0.0, 0.0, -1.0, 1.0);
    }

    // Right-hand coordinate
    inline static Matrix4 CreatePerspectiveFov(T fov, T width, T height, T near, T far)
    {
        T scale_y = Math::Cot(fov * 0.5);
        T scale_x = scale_y * height / width;
        T half_w = near / scale_x;
        T half_h = near / scale_y;
        return CreatePerspectiveOffCenter(-half_w, half_w, -half_h, half_h, near, far);
    }

    // Create "Simple" View-Projection Matrix from Chapter 6
    inline static Matrix4 CreateSimpleViewProj(T w, T h)
    {
        return Matrix4(
            (T)2.0/w,      0.0,  0.0, 0.0,
                 0.0, (T)2.0/h,  0.0, 0.0,
                 0.0,      0.0, -1.0, 1.0,
                 0.0,      0.0,  0.0, 1.0);
    }

    inline static Matrix4 FromQuaternion(const Quaternion<T>& q)
    {
        Matrix4 res;

        res.m00_ = 1.0 - 2.0*q.y_*q.y_ - 2.0*q.z_*q.z_;
        res.m10_ = 2.0*q.x_*q.y_ + 2.0*q.w_*q.z_;
        res.m20_ = 2.0*q.x_*q.z_ - 2.0*q.w_*q.y_;
        res.m30_ = 0.0;

        res.m01_ = 2.0*q.x_*q.y_ - 2.0*q.w_*q.z_;
        res.m11_ = 1.0 - 2.0*q.x_*q.x_ - 2.0*q.z_*q.z_;
        res.m21_ = 2.0*q.y_*q.z_ + 2.0*q.w_*q.x_;
        res.m31_ = 0.0;

        res.m02_ = 2.0*q.x_*q.z_ + 2.0*q.w_*q.y_;
        res.m12_ = 2.0*q.y_*q.z_ - 2.0*q.w_*q.x_;
        res.m22_ = 1.0 - 2.0*q.x_*q.x_ - 2.0*q.y_*q.y_;
        res.m32_ = 0.0;

        res.m03_ = 0.0;
        res.m13_ = 0.0;
        res.m23_ = 0.0;
        res.m33_ = 1.0;

        return res;
    }

public:
    inline friend Matrix4 operator*(const Matrix4& a, const Matrix4& b)
    {
        Matrix4 res;
        res.m00_ = a.m00_ * b.m00_ + a.m01_ * b.m10_ + a.m02_ * b.m20_ + a.m03_ * b.m30_;
        res.m01_ = a.m00_ * b.m01_ + a.m01_ * b.m11_ + a.m02_ * b.m21_ + a.m03_ * b.m31_;
        res.m02_ = a.m00_ * b.m02_ + a.m01_ * b.m12_ + a.m02_ * b.m22_ + a.m03_ * b.m32_;
        res.m03_ = a.m00_ * b.m03_ + a.m01_ * b.m13_ + a.m02_ * b.m23_ + a.m03_ * b.m33_;
        res.m10_ = a.m10_ * b.m00_ + a.m11_ * b.m10_ + a.m12_ * b.m20_ + a.m13_ * b.m30_;
        res.m11_ = a.m10_ * b.m01_ + a.m11_ * b.m11_ + a.m12_ * b.m21_ + a.m13_ * b.m31_;
        res.m12_ = a.m10_ * b.m02_ + a.m11_ * b.m12_ + a.m12_ * b.m22_ + a.m13_ * b.m32_;
        res.m13_ = a.m10_ * b.m03_ + a.m11_ * b.m13_ + a.m12_ * b.m23_ + a.m13_ * b.m33_;
        res.m20_ = a.m20_ * b.m00_ + a.m21_ * b.m10_ + a.m22_ * b.m20_ + a.m23_ * b.m30_;
        res.m21_ = a.m20_ * b.m01_ + a.m21_ * b.m11_ + a.m22_ * b.m21_ + a.m23_ * b.m31_;
        res.m22_ = a.m20_ * b.m02_ + a.m21_ * b.m12_ + a.m22_ * b.m22_ + a.m23_ * b.m32_;
        res.m23_ = a.m20_ * b.m03_ + a.m21_ * b.m13_ + a.m22_ * b.m23_ + a.m23_ * b.m33_;
        res.m30_ = a.m30_ * b.m00_ + a.m31_ * b.m10_ + a.m32_ * b.m20_ + a.m33_ * b.m30_;
        res.m31_ = a.m30_ * b.m01_ + a.m31_ * b.m11_ + a.m32_ * b.m21_ + a.m33_ * b.m31_;
        res.m32_ = a.m30_ * b.m02_ + a.m31_ * b.m12_ + a.m32_ * b.m22_ + a.m33_ * b.m32_;
        res.m33_ = a.m30_ * b.m03_ + a.m31_ * b.m13_ + a.m32_ * b.m23_ + a.m33_ * b.m33_;

        return res;
    }

    inline Matrix4& operator*=(const Matrix4& rhs)
    {
        *this = *this * rhs;
        return *this;
    }

public:
    static const Matrix4 ZERO;
    static const Matrix4 IDENTITY;
};

template<typename T> const Matrix4<T> Matrix4<T>::ZERO =
Matrix4<T>(
    0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0
);

template<typename T> const Matrix4<T> Matrix4<T>::IDENTITY =
Matrix4<T>(
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
);

template <class T>
using Mat4 = Matrix4<T>;
using Mat4f = Matrix4<float>;
using Mat4d = Matrix4<double>;
