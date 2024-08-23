#include "Lerp.h"

namespace Crystal { namespace Math
{

float lerp(float a, float b, float f)
{
    return a * (1.0 - f) + (b * f);
}

}
}