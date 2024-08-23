#pragma once

#include "imgui.h"

namespace Crystal { namespace Math
{

struct Vector2
{
    float x, y;

    Vector2(void);
    Vector2(float x, float y);
    Vector2(const ImVec2& vec);

    operator ImVec2(void) const;

    Vector2 operator+(const Vector2& other) const;
    Vector2 operator-(const Vector2& other) const;
    Vector2 operator*(float scalar) const;
    Vector2 operator/(float scalar) const;

    Vector2& operator+=(const Vector2& other);
    Vector2& operator-=(const Vector2& other);
    Vector2& operator*=(float scalar);
    Vector2& operator/=(float scalar);

    bool operator==(const Vector2& other) const;
    bool operator!=(const Vector2& other) const;
};

Vector2 operator*(float scalar, const Vector2& vec);

}
}