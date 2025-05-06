//
// Created by xiami on 2025/5/5.
//
// perlinnoise.h
#pragma once
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>

class PerlinNoise
{
public:
    explicit PerlinNoise(unsigned seed = std::random_device{}())
    {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);
        std::mt19937 gen(seed);
        std::shuffle(p.begin(), p.end(), gen);
        p.insert(p.end(), p.begin(), p.end());
    }

    // 返回范围 [0,1] 的噪声值
    double noise(double x) const
    {
        int xi  = static_cast<int>(std::floor(x)) & 255;
        double xf = x - std::floor(x);
        double u  = fade(xf);

        double a = p[xi];
        double b = p[xi + 1];

        return lerp(u, grad(a, xf), grad(b, xf - 1));
    }

private:
    std::vector<int> p;

    static double fade(double t)
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static double lerp(double t, double a, double b)
    {
        return a + t * (b - a);
    }

    static double grad(int hash, double x)
    {
        return ((hash & 1) ? -x : x);
    }
};