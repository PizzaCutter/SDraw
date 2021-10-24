#pragma once
#include <cmath>

struct Vector2D
{
    float x;
    float y;

    Vector2D()
    {
       x = y = 0.f; 
    }

    Vector2D(float inX, float inY)
    {
        x = inX;
        y = inY;
    }

    void Normalize()
    {
        float size = std::sqrtf(x*x + y*y);
        if (size == 0.0f)
        {
            x = 0.0f;
            y = 0.0f;
        }
        
        x /= size;
        y /= size;
    }

    Vector2D operator+ (const Vector2D& rhs) const
    {
        return Vector2D {x + rhs.x, y + rhs.y };
    }

    static const Vector2D ZeroVector;
    static const Vector2D OneVector;
};

struct Transform
{
    Vector2D Position = Vector2D::ZeroVector;
    Vector2D Scale = Vector2D::OneVector;
};

static float GetRandomNormalizedFloat()
{
    float randomFloat = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return (randomFloat * 2)- 1.0f;
}
