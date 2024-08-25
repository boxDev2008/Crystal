#include "Vector2.h"
#include <math.h>

namespace Crystal { namespace Math
{

Vector2::Vector2() : x(0), y(0) {}
Vector2::Vector2(float x, float y) : x(x), y(y) {}
Vector2::Vector2(const ImVec2& vec) : x(vec.x), y(vec.y) {}

Vector2::operator ImVec2() const
{
    return ImVec2(x, y);
}

Vector2 Vector2::operator+(const Vector2& other) const
{
    return Vector2(x + other.x, y + other.y);
}

Vector2 Vector2::operator-(const Vector2& other) const
{
    return Vector2(x - other.x, y - other.y);
}

Vector2 Vector2::operator*(float scalar) const
{
    return Vector2(x * scalar, y * scalar);
}

Vector2 Vector2::operator/(float scalar) const
{
    return Vector2(x / scalar, y / scalar);
}

Vector2& Vector2::operator+=(const Vector2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

Vector2& Vector2::operator-=(const Vector2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2& Vector2::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2& Vector2::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

bool Vector2::operator==(const Vector2& other) const
{
    return x == other.x && y == other.y;
}

bool Vector2::operator!=(const Vector2& other) const
{
    return !(*this == other);
}

float Vector2::Angle(const Vector2& from, const Vector2& to)
{
    return 180.0f - Dot(from, to) * 180.0f;
}    

float Vector2::SignedAngle(const Vector2& from, const Vector2& to)
{
    return Dot(from, to) * 180.0f;
}

float Vector2::Distance(const Vector2& lhs, const Vector2& rhs)
{
    return sqrtf((lhs.x - rhs.x) * (lhs.x - rhs.x) + (lhs.y - rhs.y) * (lhs.y - rhs.y));
}

float Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
{
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

Vector2 Vector2::Lerp(const Vector2& a, const Vector2& b, float t)
{
    return a * (1 - t) + b * t;
}

Vector2 Vector2::FromAngle(float angle)
{
    float x = cosf(angle);
    float y = sinf(angle);
    return {x - y, y + x};
}

Vector2 Vector2::Max(const Vector2& lhs, const Vector2& rhs)
{
    return {lhs.x > rhs.x ? lhs.x : rhs.x, lhs.y > rhs.y ? lhs.y : rhs.y};
}

Vector2 Vector2::Min(const Vector2& lhs, const Vector2& rhs)
{
    return {lhs.x < rhs.x ? lhs.x : rhs.x, lhs.y < rhs.y ? lhs.y : rhs.y};
}

float Vector2::Heading(const Vector2& vector) { return atan2f(vector.y, vector.x); }
float Vector2::Magnitude(const Vector2& vector) { return sqrtf(vector.x * vector.x + vector.y * vector.y); }
float Vector2::SqrMagnitude(const Vector2& vector) { return vector.x * vector.x + vector.y * vector.y; }
Vector2 Vector2::Normalize(const Vector2& vector)
{
    return vector / Vector2::Magnitude(vector);
}

Vector2 operator*(float scalar, const Vector2& vec)
{
    return Vector2(vec.x * scalar, vec.y * scalar);
}

}
}