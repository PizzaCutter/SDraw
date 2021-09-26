#include "SEngine.h"

#include <iostream>
#include <algorithm>

static float paddleMovementTimeFromTopToBottom = 3.f;
static float paddleHeightScreenPercentage = 0.1f;
static float paddleWidthScreenPercentage = 0.015f;
static float paddleBorderOffsetScreenPercentage = 0.02f;

static float minBallMovementTimeFromLeftToRight = 5.0f;
static float maxBallMovementTimeFromLeftToRight = 2.0f;
static float ballSizeScreenPercentage = 0.01f;

static float middleLineGridPointSizePercentage = 0.01f;
static float middleLineGridPointOffsetPercentage = 0.075f;

static float scoreXOffsetPercentage = 0.05f;
static float scoreYOffsetPercentage = 0.025f;

#define ARROW_UP 0x26
#define ARROW_DOWN 0x28

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
    Vector2D size;
    Direction direction = Direction::None;
    float speed;
    int score;

    void UpdatePosition(float deltaTime)
    {
        // Update position based on movement direction
        if (direction == Direction::Down)
        {
            position.y += speed * deltaTime;
        }else if(direction == Direction::Up)
        {
            position.y -= speed * deltaTime;
        }

        // Clamp the position between screen coordinates
        position.y = std::clamp(position.y, 0.0f,(float)Height - size.y);        
    }

    void Draw(float deltaTime)
    {
        DrawRectangle(position.x, position.y, size.x, size.y, White);
    }
};

struct Ball
{
    Vector2D position;
    Vector2D size;
    Vector2D direction;
    float speed;

    bool isOverlapping;

    bool IsOverlappingWithPaddle(const Paddle& paddle)
    {
         if (position.x + size.x >= paddle.position.x && position.x <= paddle.position.x + paddle.size.x)
         {
             if (position.y + size.y >= paddle.position.y && position.y <= paddle.position.y + paddle.size.y)
             {
                return true;     
             }
         }
         return false;
    }

    void UpdatePosition(float deltaTime, const Paddle& leftPaddle, const Paddle& rightPaddle)
    {
        isOverlapping = false;
        
        position.x += direction.x * speed * deltaTime;
        position.y += direction.y * speed * deltaTime;

        if (position.y <= 0.0f)
        {
           direction.y *= -1.0f;
           position.y = 0.0f;
        }else if (position.y >= Height - size.y)
        {
            direction.y *= -1.0f;
            position.y = Height - size.y;
        }

        const float epsilon = 0.001f;
        if (direction.x < epsilon && IsOverlappingWithPaddle(leftPaddle)) 
        {
            direction.x *= -1.0f;
        }
        if (direction.x > epsilon && IsOverlappingWithPaddle(rightPaddle))
        {
            direction.x *= -1.0f;
        }

        if (position.x <= 0)
        {
            direction.x *= -1.0f;
            position.x = 0;
        }else if (position.x >= Width - size.x)
        {
            direction.x *= -1.0f;
            position.x = Width - size.x;
        }
    }

    void Draw(float deltaTime)
    {
        Color colorToUse = isOverlapping ? Red : White;
        DrawRectangle(position.x, position.y, size.x, size.y, colorToUse);
    }
};

Paddle paddles[2];
Ball ball;

void Start()
{
    const float paddleOffset = Width * paddleBorderOffsetScreenPercentage;
    const float paddleHeight = Height * paddleHeightScreenPercentage;
    const float paddleWidth = Width * paddleWidthScreenPercentage;
    const float paddleSpeed = (Height - paddleHeight) / paddleMovementTimeFromTopToBottom;
    
    const float ballSize = Width * ballSizeScreenPercentage;
    const float ballSpeed = (Width - ballSize) / minBallMovementTimeFromLeftToRight;
    
    Paddle leftPaddle = {};
    leftPaddle.size = Vector2D{paddleWidth, paddleHeight};
    leftPaddle.position.x = paddleOffset;
    leftPaddle.position.y = Height / 2.0f - (leftPaddle.size.y / 2.0f);
    leftPaddle.speed = paddleSpeed;
    paddles[0] = leftPaddle;
    
    Paddle rightPaddle = {};
    rightPaddle.size = Vector2D{paddleWidth, paddleHeight};
    rightPaddle.position.x = Width - rightPaddle.size.x - paddleOffset;
    rightPaddle.position.y = Height / 2.0f - (rightPaddle.size.y / 2.0f);
    rightPaddle.speed = paddleSpeed;
    paddles[1] = rightPaddle;

    ball = {};
    ball.position = Vector2D{Width / 2, Height / 2};
    ball.direction = Vector2D{0.5f, 0.5f};
    ball.size = Vector2D{ballSize, ballSize};
    ball.speed = ballSpeed;
}

void DrawGridLine()
{
    float gridPointSize = Width * middleLineGridPointSizePercentage;
    float gridPointOffset = Height * middleLineGridPointOffsetPercentage;

    float yPos = 0.0f;
    while (yPos <= Height)
    {
        DrawRectangle((Width / 2.f) - gridPointSize / 2.f, yPos, gridPointSize, gridPointSize, White);
        yPos += gridPointOffset; 
    }
}

void UpdateGame(float deltaTime)
{
    Clear(Black);

    Paddle& leftPaddle = paddles[0];
    Paddle& rightPaddle = paddles[1];
    
    if (IsKeyDown('W'))
    {
        leftPaddle.direction = Direction::Up;
    }else if(IsKeyDown('S'))
    {
        leftPaddle.direction = Direction::Down;
    }
    if (IsKeyDown(ARROW_UP))
    {
        rightPaddle.direction = Direction::Up;
    }else if (IsKeyDown(ARROW_DOWN))
    {
        rightPaddle.direction = Direction::Down;
    }
    
    for (Paddle& paddle : paddles)
    {
        paddle.UpdatePosition(deltaTime);
    }

    ball.UpdatePosition(deltaTime, paddles[0], paddles[1]);

    // Rendering 
    DrawGridLine();
    for (Paddle& paddle : paddles)
    {
        paddle.Draw(deltaTime);
    }
    
    leftPaddle = paddles[0];
    
    const float xOffset = Width * scoreXOffsetPercentage;
    const float yOffset = Height * scoreYOffsetPercentage;
    DrawString((Width / 2) - xOffset, yOffset, std::to_string(leftPaddle.score), LightGray, 10);
    DrawString((Width / 2) + xOffset, yOffset, std::to_string(rightPaddle.score), LightGray, 10);
   
    ball.Draw(deltaTime);
}