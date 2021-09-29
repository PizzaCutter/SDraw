#pragma once
#include <cmath>

struct Vector2D
{
    float x;
    float y;

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
};
