#include "SEngine.h"

#include <iostream>

static float maxSpeed = 0.2f;
static float speed = maxSpeed;
static float xPos = 0;


enum class Direction
{
    None, Up, Down
};

struct Vector2D
{
    float x;
    float y;
};

struct Paddle
{
    Vector2D position;
    Vector2D size = {5.0f, 20.0f};
    Direction direction = Direction::None; 
};

Paddle paddles[2];

void Start()
{
    const float paddleOffset = 2.0f;
    
    Paddle leftPaddle = {};
    leftPaddle.position.x = paddleOffset;
    leftPaddle.position.y = Height / 2.0f - (leftPaddle.size.y / 2.0f);
    paddles[0] = leftPaddle;
    
    Paddle rightPaddle = {};
    rightPaddle.position.x = Width - rightPaddle.size.x - paddleOffset;
    rightPaddle.position.y = Height / 2.0f - (rightPaddle.size.y / 2.0f);
    paddles[1] = rightPaddle;
}

void UpdateGame()
{
    Clear(Black);

    for (Paddle& paddle : paddles)
    {
        const Vector2D& position = paddle.position;
        const Vector2D& size = paddle.size;
        DrawRectangle(static_cast<int>(position.x), static_cast<int>(position.y), static_cast<int>(size.x), static_cast<int>(size.y), White);
    }
    
    // xPos += speed * 0.16f;
    // SetPixel((int)xPos, Height / 2, Yellow);
    //
    // if ((int)xPos >= Width && speed > 0)
    // {
    //     speed = -maxSpeed;
    // }else if((int)xPos <= 0 && speed < 0.0f) 
    // {
    //     speed = maxSpeed;
    // }

}