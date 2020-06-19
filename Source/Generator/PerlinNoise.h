#pragma once
#include <random>
#include <iterator>
#include <Urho3D/Math/MathDefs.h>

using namespace Urho3D;

class PerlinNoise
{
private:

    std::uint8_t randomNumbers[512];

    static double Fade(double t) noexcept
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static double Lerp(double t, double a, double b) noexcept
    {
        return a + t * (b - a);
    }

    static double Grad(std::uint8_t hash, double x, double y, double z) noexcept
    {
        const std::uint8_t h = hash & 15;
        const double u = h < 8 ? x : y;
        const double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

public:

    explicit PerlinNoise(int seed)
    {
        reseed(seed);
    }

    explicit PerlinNoise()
    {
    }

    void reseed(int seed)
    {
        SetRandomSeed(seed);
        for (size_t i = 0; i < 512; ++i)
        {
            randomNumbers[i] = static_cast<std::uint8_t>(i);
            randomNumbers[i] = Random();
        }

//        std::begin(randomNumbers), std::begin(randomNumbers) + 512, std::default_random_engine(seed);
    }

    double noise(double x) const
    {
        return noise(x, 0.0, 0.0);
    }

    double noise(double x, double y) const
    {
        return noise(x, y, 0.0);
    }

    double noise(double x, double y, double z) const
    {
        const int X = static_cast<int>(Floor(x)) & 511;
        const int Y = static_cast<int>(Floor(y)) & 511;
        const int Z = static_cast<int>(Floor(z)) & 511;

        x -= Floor(x);
        y -= Floor(y);
        z -= Floor(z);

        const double u = Fade(x);
        const double v = Fade(y);
        const double w = Fade(z);

        const int A = randomNumbers[X] + Y, AA = randomNumbers[A] + Z, AB = randomNumbers[A + 1] + Z;
        const int B = randomNumbers[X + 1] + Y, BA = randomNumbers[B] + Z, BB = randomNumbers[B + 1] + Z;

        return Lerp(w, Lerp(v, Lerp(u, Grad(randomNumbers[AA], x, y, z),
                                    Grad(randomNumbers[BA], x - 1, y, z)),
                            Lerp(u, Grad(randomNumbers[AB], x, y - 1, z),
                                 Grad(randomNumbers[BB], x - 1, y - 1, z))),
                    Lerp(v, Lerp(u, Grad(randomNumbers[AA + 1], x, y, z - 1),
                                 Grad(randomNumbers[BA + 1], x - 1, y, z - 1)),
                         Lerp(u, Grad(randomNumbers[AB + 1], x, y - 1, z - 1),
                              Grad(randomNumbers[BB + 1], x - 1, y - 1, z - 1))));
    }

    double octaveNoise(double x, int octaves) const
    {
        double result = 0.0;
        double amp = 1.0;

        for (int i = 0; i < octaves; ++i)
        {
            result += noise(x) * amp;
            x *= 2.0;
            amp *= 0.5;
        }

        return result;
    }

    double octaveNoise(double x, double y, int octaves) const
    {
        double result = 0.0;
        double amp = 1.0;

        for (std::int32_t i = 0; i < octaves; ++i)
        {
            result += noise(x, y) * amp;
            x *= 2.0;
            y *= 2.0;
            amp *= 0.5;
        }

        return result;
    }

    double octaveNoise(double x, double y, double z, int octaves) const
    {
        double result = 0.0;
        double amp = 1.0;

        for (int i = 0; i < octaves; ++i)
        {
            result += noise(x, y, z) * amp;
            x *= 2.0;
            y *= 2.0;
            z *= 2.0;
            amp *= 0.5;
        }

        return result;
    }

    double noise0_1(double x) const
    {
        return noise(x) * 0.5 + 0.5;
    }

    double noise0_1(double x, double y) const
    {
        return noise(x, y) * 0.5 + 0.5;
    }

    double noise0_1(double x, double y, double z) const
    {
        return noise(x, y, z) * 0.5 + 0.5;
    }

    double octaveNoise0_1(double x, int octaves) const
    {
        return octaveNoise(x, octaves) * 0.5 + 0.5;
    }

    double octaveNoise0_1(double x, double y, int octaves) const
    {
        return octaveNoise(x, y, octaves) * 0.5 + 0.5;
    }

    double octaveNoise0_1(double x, double y, double z, int octaves) const
    {
        return octaveNoise(x, y, z, octaves) * 0.5 + 0.5;
    }
};
