#pragma once

#include <random>
#include "vector.h"

class Random
{
public:
    static void Init() {
        std::random_device rd;
        Seed(rd());
    }

    static void Seed(unsigned int seed) {
        generator__.seed(seed);
    }

    template<typename T>
    static T GetValue()
    {
        return GetValueRange(0.0, 1.0);
    }

    template<>
    static float GetValue()
    {
        return GetValueRange<float>(0.0f, 1.0f);
    }

    template<typename T>
    static T GetValueRange(T min, T max)
    {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(generator__);
    }

    template<>
    static int GetValueRange<int>(int min, int max)
    {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(generator__);
    }

    template <typename T>
    static Vec2<T> GetVector(const Vec2<T> &min, const Vec2<T> &max)
    {
        auto r = Vec2<T>(GetValue<T>(), GetValue<T>());
        return min + (max - min) * r;
    }

    template<typename T>
    static Vec3<T> GetVector(const Vec3<T>& min, const Vec3<T>& max)
    {
        auto r = Vec3<T>(GetValue<T>(), GetValue<T>(), GetValue<T>());
        return min + (max - min) * r;
    }

private:
    static std::mt19937 generator__;
};

std::mt19937 Random::generator__;
