#pragma once

#include "math.h"
#include "vector.h"

template <typename T> struct Vector3;

template<typename T>
struct Quaternion
{
#pragma pack(push, 1)
    union {
        struct {
            T x_;
            T y_;
            T z_;
            T w_;
        };
        T data_[4];
    };
#pragma pack(pop)

    inline Quaternion()
    {
        *this = Quaternion::IDENTITY;
    }

    inline explicit Quaternion(T x, T y, T z, T w) :
        x_(x), y_(y), z_(z), w_(w)
    {
    }

    inline explicit Quaternion(const Vector3<T>& axis, T angle)
    {
        T scalar = Math::Sin(angle / 2.0f);
        x_ = axis.x_ * scalar;
        y_ = axis.y_ * scalar;
        z_ = axis.z_ * scalar;
        w_ = Math::Cos(angle / 2.0f);
    }

    inline T LengthSq() const
    {
        return (x_*x_ + y_*y_ + z_*z_ + w_*w_);
    }

    inline T Length() const
    {
        return Math::Sqrt(LengthSq());
    }

    inline void Set(T x, T y, T z, T w)
    {
        x_ = x;
        y_ = y;
        z_ = z;
        w_ = w;
    }

    inline void Conjugate()
    {
        x_ *= -1.0;
        y_ *= -1.0;
        z_ *= -1.0;
    }

    inline void Normalize()
    {
        T length = Length();
        x_ /= length;
        y_ /= length;
        z_ /= length;
        w_ /= length;
    }

    inline static Quaternion Normalize(const Quaternion& q)
    {
        Quaternion retVal = q;
        retVal.Normalize();
        return retVal;
    }

    inline static Quaternion Lerp(const Quaternion& a, const Quaternion& b, T f)
    {
        Quaternion retVal;
        retVal.x_ = Math::Lerp(a.x_, b.x_, f);
        retVal.y_ = Math::Lerp(a.y_, b.y_, f);
        retVal.z_ = Math::Lerp(a.z_, b.z_, f);
        retVal.w_ = Math::Lerp(a.w_, b.w_, f);
        retVal.Normalize();
        return retVal;
    }

    inline static T Dot(const Quaternion& a, const Quaternion& b)
    {
        return a.x_ * b.x_ + a.y_ * b.y_ + a.z_ * b.z_ + a.w_ * b.w_;
    }

    inline static Quaternion Slerp(const Quaternion& a, const Quaternion& b, T f)
    {
        T raw_cosm = Quaternion::Dot(a, b);
        T cosom = -raw_cosm;

        if (raw_cosm >= 0.0) {
            cosom = raw_cosm;
        }

        T scale0, scale1;

        if (cosom < 0.9999) {
            const T omega = Math::Acos(cosom);
            const T inv_sin = 1.0 / Math::Sin(omega);
            scale0 = Math::Sin((1.0 - f) * omega) * inv_sin;
            scale1 = Math::Sin(f * omega) * inv_sin;
        }
        else {
            scale0 = 1.0 - f;
            scale1 = f;
        }

        if (raw_cosm < 0.0) {
            scale1 = -scale1;
        }

        Quaternion ret;
        ret.x_ = scale0 * a.x_ + scale1 * b.x_;
        ret.y_ = scale0 * a.y_ + scale1 * b.y_;
        ret.z_ = scale0 * a.z_ + scale1 * b.z_;
        ret.w_ = scale0 * a.w_ + scale1 * b.w_;
        ret.Normalize();
        return ret;
    }

    inline static Quaternion<T> Concatenate(const Quaternion& q, const Quaternion& p)
    {
        Quaternion ret;
        Vector3<T> qv(q.x_, q.y_, q.z_);
        Vector3<T> pv(p.x_, p.y_, p.z_);
        Vector3<T> new_vec = p.w_ * qv + q.w_ * pv + Vector3<T>::Cross(pv, qv);
        ret.x_ = new_vec.x_;
        ret.y_ = new_vec.y_;
        ret.z_ = new_vec.z_;
        ret.w_ = p.w_ * q.w_ - Vector3<T>::Dot(pv, qv);

        return ret;
    }

    static const Quaternion ZERO;
    static const Quaternion IDENTITY;
};

template<typename T> const Quaternion<T> Quaternion<T>::ZERO = Quaternion<T>(0, 0, 0, 0);
template<typename T> const Quaternion<T> Quaternion<T>::IDENTITY = Quaternion<T>(0, 0, 0, 1);

template <class T>
using Quat = Quaternion<T>;
using Quatf = Quaternion<float>;
using Quatd = Quaternion<double>;
