#include "Math.h"
#include <math.h>

namespace Crystal { namespace Math
{

float Lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

float Clamp(float value, float min, float max)
{
    return value < min ? min : value > max ? max : value;
}

float Clamp01(float value)
{
    return value < 0.0f ? 0.0f : value > 1.0f ? 1.0f : value;
}

}
}