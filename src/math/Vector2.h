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

    static float Angle(const Vector2& from, const Vector2& to);
    static float SignedAngle(const Vector2& from, const Vector2& to);
    static float Distance(const Vector2& lhs, const Vector2& rhs);
    static float Dot(const Vector2& lhs, const Vector2& rhs);
    static Vector2 Lerp(const Vector2& a, const Vector2& b, float t);
    static Vector2 FromAngle(float angle);
    static Vector2 Max(const Vector2& lhs, const Vector2& rhs);
    static Vector2 Min(const Vector2& lhs, const Vector2& rhs);
    static float Heading(const Vector2& vector);
    static float Magnitude(const Vector2& vector);
    static float SqrMagnitude(const Vector2& vector);
    static Vector2 Vector2::Normalize(const Vector2& vector);
};

Vector2 operator*(float scalar, const Vector2& vec);

}
}