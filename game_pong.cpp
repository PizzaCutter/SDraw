#include "SEngine.h"
#include "SMath.h"

#include <iostream>
#include <algorithm>

static float paddleMovementTimeFromTopToBottom = 3.f;
static float paddleHeightScreenPercentage = 0.1f;
static float paddleWidthScreenPercentage = 0.015f;
static float paddleBorderOffsetScreenPercentage = 0.02f;

static float minBallMovementTimeFromLeftToRight = 5.0f;
static float ballSizeScreenPercentage = 0.01f;

static float middleLineGridPointSizePercentage = 0.01f;
static float middleLineGridPointOffsetPercentage = 0.075f;

static float scoreXOffsetPercentage = 0.05f;
static float scoreYOffsetPercentage = 0.025f;

static float ballIdleTimer = 1.5f;

static int32 requiredScoreToWin = 3;

static float titleSizeScreenPercentage = 0.03f;
static float startGameMessageScreenPercentage = 0.01f;
static float scoreScreenPercentage = 0.01f;

// Music Timing
constexpr int MsPerMinute = 60000;
constexpr int BPM = 160;
constexpr int BeatsPerMs = MsPerMinute / BPM;

constexpr int Duration4 =  BeatsPerMs * 1 / 1;
constexpr int Duration8 =  BeatsPerMs * 1 / 2;
constexpr int Duration10 = BeatsPerMs * 2 / 5;
constexpr int Duration16 = BeatsPerMs * 1 / 4;
constexpr int Duration20 = BeatsPerMs * 1 / 5;
constexpr int Duration32 = BeatsPerMs * 1 / 8;

#define ARROW_UP 0x26
#define ARROW_DOWN 0x28
#define SPACEBAR 0x20

void PlayBallBounce();
void PlayBallDestroyed();

enum class Direction
{
    None, Up, Down
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
        DrawFilledRectangle(position, size, White);
    }
};

struct Ball
{
    Vector2D position;
    Vector2D size;
    Vector2D direction;
    float speed;
    float idleTimer;

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
        if (idleTimer <= ballIdleTimer)
        {
            idleTimer += deltaTime;
            return;     
        }
        
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
            PlayBallBounce();
        }
        if (direction.x > epsilon && IsOverlappingWithPaddle(rightPaddle))
        {
            direction.x *= -1.0f;
            PlayBallBounce();
        }
    }

    void Draw(float deltaTime)
    {
        Color colorToUse = isOverlapping ? Red : White;
        DrawFilledRectangle(position, size, colorToUse);
    }
};

bool isInMenu = true;
Paddle paddles[2];
Ball ball;

void PlayTitleMusic()
{
    for (int n : { 0, 48, 50, 52, 50, 48, 50 }) PlayMidiNote(n, Duration8);
    for (int n : { 52, 48, 48 }) PlayMidiNote(n, Duration4);
}

void PlayBallBounce()
{
   PlayMidiNote(48, Duration8);
}

void PlayBallDestroyed()
{
   PlayMidiNote(64, Duration8); 
}

void ResetGame()
{
    const float paddleOffset = Width * paddleBorderOffsetScreenPercentage;
    const float paddleHeight = Height * paddleHeightScreenPercentage;
    const float paddleWidth = Width * paddleWidthScreenPercentage;
    const float paddleSpeed = (Height - paddleHeight) / paddleMovementTimeFromTopToBottom;
    
    const float ballSize = Width * ballSizeScreenPercentage;
    const float ballSpeed = (Width - ballSize) / minBallMovementTimeFromLeftToRight;

    Paddle& leftPaddle = paddles[0];
    leftPaddle.size = Vector2D{paddleWidth, paddleHeight};
    leftPaddle.position.x = paddleOffset;
    leftPaddle.position.y = Height / 2.0f - (leftPaddle.size.y / 2.0f);
    leftPaddle.speed = paddleSpeed;
    leftPaddle.direction = Direction::None;

    Paddle& rightPaddle = paddles[1];
    rightPaddle.size = Vector2D{paddleWidth, paddleHeight};
    rightPaddle.position.x = Width - rightPaddle.size.x - paddleOffset;
    rightPaddle.position.y = Height / 2.0f - (rightPaddle.size.y / 2.0f);
    rightPaddle.speed = paddleSpeed;
    rightPaddle.direction = Direction::None;
    
    ball.size = Vector2D{ballSize, ballSize};
    const float halfBallSize = ballSize / 2.0f; 
    ball.position = Vector2D{(Width / 2) - halfBallSize, (Height / 2) - halfBallSize};
    ball.direction = Vector2D{GetRandomNormalizedFloat(), GetRandomNormalizedFloat()};
    ball.direction.Normalize();
    ball.speed = ballSpeed;
    ball.idleTimer = 0.0f; 
}

void Start()
{
    SetApplicationName("PONG");
    
    Paddle leftPaddle = {};
    paddles[0] = leftPaddle;
    
    Paddle rightPaddle = {};
    paddles[1] = rightPaddle;

    ball = {};

    ResetGame();

}

void DrawGridLine()
{
    float gridPointSize = Width * middleLineGridPointSizePercentage;
    float gridPointOffset = Height * middleLineGridPointOffsetPercentage;

    float yPos = 0.0f;
    while (yPos <= Height)
    {
        DrawFilledRectangle(
            Vector2D{(Width / 2.f) - gridPointSize / 2.f, yPos },
           Vector2D{gridPointSize, gridPointSize}, White);
        
        yPos += gridPointOffset; 
    }
}

void CheckWinCondition()
{
    Paddle& leftPaddle = paddles[0];
    Paddle& rightPaddle = paddles[1];
    
    const float winOffset = 5;
    if (ball.position.x + ball.size.x <= -winOffset)
    {
        leftPaddle.score++;
        PlayBallDestroyed();
        ResetGame();
    }
    if (ball.position.x >= Width + winOffset)
    {
        rightPaddle.score++;
        PlayBallDestroyed();
        ResetGame(); 
    }

    if (leftPaddle.score >= requiredScoreToWin)
    {
        isInMenu = true;    
    }
    if (rightPaddle.score >= requiredScoreToWin)
    {
        isInMenu = true; 
    }
}

void UpdateMenu(float deltaTime)
{
    if (IsKeyDown(SPACEBAR))
    {
        Paddle& leftPaddle = paddles[0];
        Paddle& rightPaddle = paddles[1];
        leftPaddle.score = 0;
        rightPaddle.score = 0;
        isInMenu = false;        
    }
}

void RenderMenu(float deltaTime)
{
    Clear();
    //RenderGrid();
    
    const int32 titleTextSize = static_cast<int32>(Width * titleSizeScreenPercentage);
    const std::string gameTitle = "PONG";
    {
        const int32 xPos = Width / 2;
        const int32 yPos = static_cast<int32>(static_cast<float>(Height) * 0.1f);
        DrawString(xPos, yPos, gameTitle, Center, White, titleTextSize);
    }

    const int32 startMessageTextSize = static_cast<int32>(Width * startGameMessageScreenPercentage);
    const std::string other = "SPACEBAR TO START";
    {
        const int32 xPos = Width / 2;
        const int32 yPos = static_cast<int32>(static_cast<float>(Height) * 0.5f);
        DrawString(xPos, yPos, other, Center, White, startMessageTextSize);
    }

    Paddle leftPaddle = paddles[0];
    Paddle rightPaddle = paddles[1];

    std::string winnerText;
    if (leftPaddle.score >= requiredScoreToWin)
    {
        winnerText = "LEFT PADDLE WON";
    }
    if (rightPaddle.score >= requiredScoreToWin)
    {
        winnerText = "RIGHT PADDLE WON";
    }
    
    {
        const int32 xPos = Width / 2;
        const int32 yPos = static_cast<int32>(static_cast<float>(Height) * 0.6f);
        DrawString(xPos, yPos, winnerText, Center, White, startMessageTextSize);
    }
}

void UpdateGame(float deltaTime)
{
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
    if (IsKeyDown('R'))
    {
        ResetGame();
    }
    
    for (Paddle& paddle : paddles)
    {
        paddle.UpdatePosition(deltaTime);
    }

    ball.UpdatePosition(deltaTime, paddles[0], paddles[1]);

    CheckWinCondition();
}

void RenderGame(float deltaTime)
{
    Clear(Black);
    
    //RenderGrid();
    DrawGridLine();
    
    for (Paddle& paddle : paddles)
    {
        paddle.Draw(deltaTime);
    }

    // Render score
    {
        Paddle& leftPaddle = paddles[0];
        Paddle& rightPaddle = paddles[1];
        const float xOffset = Width * scoreXOffsetPercentage;
        const float yOffset = Height * scoreYOffsetPercentage;
        
        const int32 textSize = static_cast<int32>(Width * scoreScreenPercentage);
        // Left Paddle Score
        {
            const std::string scoreAsString = std::to_string(leftPaddle.score);
            const int32 textWidth = GetStringWidth(scoreAsString) * textSize;
            DrawString(static_cast<int32>((Width / 2) - xOffset), static_cast<int32>(yOffset), scoreAsString, Left, White, textSize);
        }
        // Right Paddle Score
        {
            const std::string scoreAsString = std::to_string(rightPaddle.score);
            const int32 textWidth = GetStringWidth(scoreAsString) * textSize;
            DrawString(static_cast<int32>((Width / 2) + xOffset - textWidth), static_cast<int32>(yOffset), scoreAsString, Left, White, textSize);
        }
    }
   
    ball.Draw(deltaTime); 
}

void Tick(float deltaTime)
{
    //PlayTitleMusic();
    
    if (isInMenu)
    {
        UpdateMenu(deltaTime);
        RenderMenu(deltaTime);
    }else
    {
        UpdateGame(deltaTime);
        RenderGame(deltaTime);
    }
}