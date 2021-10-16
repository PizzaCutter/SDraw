#include "SEngine.h"
#include "SMath.h"

#include <iostream>
#include <vector>

#define INVADER_SPEED 50.f
#define INVADER_START_DIRECTION 1.f
#define INVADER_SPRITE_CELL_SIZE 22;
#define INVADER_X_OFFSET 15.f 
#define INVADER_Y_OFFSET 10.f

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
	Vector2D Scale;
	SSprite Sprite;
	float Direction;
	float MovementSpeed;
	bool Alive;

	void UpdateInvader(float deltaTime)
	{
		Sprite.UpdateSprite(deltaTime);

		if (Direction > 0.1f)
		{
			Position.x += MovementSpeed * deltaTime;
		}else
		{
			Position.x -= MovementSpeed * deltaTime;
		}
	}

	void MoveDownRow()
	{
		Position.y += Sprite.srcImage.height;	
	}

	void InvertDirection()
	{
		Direction *= -1.0f;	
	}

	bool ReachedEnd()
	{
		if (Direction > 0.1f)
		{
			if (Position.x >= Width - Sprite.cellSizeX)
			{
				return true;
			}
		}else
		{
			if (Position.x <= 0)
			{
				return true;
			}
		}

		return false;
	}

	void DrawInvader()
	{
		DrawSprite(Sprite, Vector2D{std::truncf(Position.x), std::truncf(Position.y)}, Scale);	
	}

	SRect GetCollisionRect() const
	{
		SRect newRect;
		newRect.x = Position.x;
		newRect.y = Position.y;
		newRect.width = Sprite.cellSizeX * Scale.x;
		newRect.height = Sprite.srcImage.height * Scale.y;
		return newRect;
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
	
	SRect GetCollisionRect() const
	{
		SRect newRect;
		newRect.x = Position.x;
		newRect.y = Position.y;
		newRect.width = Size.x;
		newRect.height = Size.y; 
		return newRect;
	} 
};


PlayerSpaceship Player;
Bullet Bullet;
std::vector<Invader*> Invaders;

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

	const int32 invaderCountHorizontal = 10;
	const int32 invaderCountVertical = 3;
	for (int x = 0; x < invaderCountHorizontal; ++x)
	{
		for (int y = 0; y < invaderCountVertical; ++y)
		{
			Invader* newInvader = new Invader();
			// TODO[rsmekens]: We should just load it once using simple implementation of an asset manager
			SLoadImage("Assets/SpaceInvader/Invader_01.png", newInvader->Sprite.srcImage);
			// TODO[rsmekens]: We could store this in an asset format that then includes reference to the image?
			newInvader->Sprite.cellSizeX = INVADER_SPRITE_CELL_SIZE;
			newInvader->MovementSpeed = INVADER_SPEED;
			newInvader->Direction = INVADER_START_DIRECTION;
			const float xPos = x * INVADER_X_OFFSET;
			const float yPos = y * INVADER_Y_OFFSET;
			newInvader->Position = Vector2D{xPos, yPos};
			newInvader->Scale = Vector2D::OneVector;
			newInvader->Scale = Vector2D{0.5f, 0.5f};
			newInvader->Alive = true;
			Invaders.push_back(newInvader);
		}
	}
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

	bool reachedEnd = false;
	for (Invader* invader : Invaders)
	{
		if (invader == nullptr)
		{
			continue;
		}

		if (invader->Alive == false)
		{
			continue;
		}

		// TODO[rsmekens]: do bullet vs invader collision checks here
		SRect invaderRect = invader->GetCollisionRect();
		SRect bulletRect = Bullet.GetCollisionRect();
		if (invaderRect.IsRectangleOverlapping(bulletRect))
		{
			invader->Alive = false;
			Bullet.Position = Vector2D{-100.f, -100.f};
			continue;
		}
		
		if (invader->ReachedEnd())
		{
			reachedEnd = true;
		}
	}

	for (Invader* invader : Invaders)
	{
		if (invader == nullptr)
		{
			continue;
		}

		if (invader->Alive == false)
		{
			continue;
		}

		if (reachedEnd)
		{
			invader->MoveDownRow();
			invader->InvertDirection();
		}

		invader->UpdateInvader(deltaTime);
		invader->DrawInvader();
	}
}