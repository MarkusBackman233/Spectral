#include "MathFunctions.h"
#include <random>

float Math::Random(float min, float max)
{
    static std::random_device rd;
    static std::mt19937 generator(rd());

    std::uniform_real_distribution<float> dist(min, max);

    return dist(generator);
}
