#include "SEngine.h"
#include "SMath.h"

#include <iostream>

#define ARROW_LEFT 0x25
#define ARROW_RIGHT 0x27
#define SPACEBAR 0x20

static float BulletSpeed = 100.0f;

// Define an image
struct PlayerSpaceship
{
	SImage PlayerSprite;
	Vector2D Position;
	float Speed;

	void UpdatePlayer(float deltaTime)
	{
		if (IsKeyDown(ARROW_LEFT))
		{
			Position.x -= Speed * deltaTime;
		}
		if (IsKeyDown(ARROW_RIGHT))
		{
			Position.x += Speed * deltaTime;		
		}
	}

	void DrawPlayer()
	{
		DrawImage(PlayerSprite, Vector2D{Position.x - PlayerSprite.GetHalfWidth(), Position.y - PlayerSprite.GetHalfHeight()});
	}
};

struct Invader
{
	Vector2D Position;
	SSprite Sprite;
	float Direction;
	float MovementSpeed;

	void UpdateInvader(float deltaTime)
	{
		Sprite.UpdateSprite(deltaTime);

		if (Direction > 0.1f)
		{
			Position.x += MovementSpeed * deltaTime;
			if (Position.x >= Width - Sprite.cellSizeX)
			{
				Direction = -1.0f;
			}
		}else
		{
			Position.x -= MovementSpeed * deltaTime;
			if (Position.x <= 0)
			{
				Direction = 1.0f;	
			}
		}
	}

	void DrawInvader()
	{
		DrawSprite(Sprite, Position);	
	}
};

struct Bullet
{
	Vector2D Position;
	Vector2D Size;
	float Velocity;
	
	void UpdateBullet(float deltaTime)
	{
		Position.y += Velocity * deltaTime;

		// We went outside of screen boundary
		if (Position.y <= -Size.y)
		{
			Velocity = 0.0f;
		}
	}
	void DrawBullet()
	{
		DrawRectangle(Position, Size, White);	
	}
};


PlayerSpaceship Player;
Bullet Bullet;
Invader Invader;

// Space invader is using these window settings 
// static constexpr int32 Width = 160 * 2; // 160
// static constexpr int32 Height = 120 * 2; // 120
// static constexpr int32 PixelScale = 4;

void Start()
{
	// Load the image
	Player = {};
	SLoadImage("Assets/SpaceInvader/Spaceship.png", Player.PlayerSprite);
	Player.Position = Vector2D{Cast<float>(Width / 2), Cast<float>(Height - 10)};
	Player.Speed = 100.0f;

	Bullet = {};
	Bullet.Position = Vector2D{-100.f, -100.f};
	Bullet.Size = Vector2D{1.f, 5.f};

	Invader = {};
	Invader.Position = Vector2D{GetHalfWidth(), GetHalfHeight()};
	SLoadImage("Assets/SpaceInvader/Invader_01.png", Invader.Sprite.srcImage);
	Invader.Sprite.cellSizeX = 22;
	Invader.MovementSpeed = 50.f;
	Invader.Direction = 1.0f;
}

void CheckFireInput()
{
	if (IsKeyDown(SPACEBAR) && Bullet.Velocity == 0.0f) // TODO[rsmekens]: create nearly zero function
	{
		Bullet.Position = Player.Position;
		Bullet.Position.y -= Bullet.Size.y;
		Bullet.Velocity = -BulletSpeed;
	}
}

void Tick(float deltaTime)
{
	Clear();
	RenderGrid();

	Player.UpdatePlayer(deltaTime);
	Player.DrawPlayer();

	Bullet.UpdateBullet(deltaTime);
	Bullet.DrawBullet();
	
	CheckFireInput();

	Invader.UpdateInvader(deltaTime);
	Invader.DrawInvader();
}