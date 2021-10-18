#include "SEngine.h"
#include "SMath.h"

#include <iostream>
#include <map>
#include <vector>

static constexpr float INVADER_SPEED = 50.f;
static constexpr float  INVADER_START_DIRECTION = 1.f;
static constexpr float  INVADER_SPRITE_CELL_SIZE = 22;
static constexpr float  INVADER_X_OFFSET = 15.f;
static constexpr float  INVADER_Y_OFFSET = 10.f;

#define ARROW_LEFT 0x25
#define ARROW_RIGHT 0x27
#define SPACEBAR 0x20

static int32 idCounter = 0;
static int32 NewId() { return idCounter++; }
static int32 assetIdCounter = 0;
static int32 NewAssetId() { return assetIdCounter++; }

struct Attributes
{
	float SPEED;
};

struct Transform
{
	Vector2D Position = Vector2D::ZeroVector;
	Vector2D Scale = Vector2D::OneVector;
};

std::map<int32, Transform> transformArray;
std::map<int32, Attributes> attributesArray;

std::map<int32, class Renderable_Image> renderableImagesArray;
std::map<int32, class Renderable_Square> renderableSquareArray;

std::map<int32, class PlayerControl> playerControlArray;
std::vector<int32> bulletArray;

std::map<int32, SImage> imageAssetArray;

int32 playerEntityId;

int32 GetAssetId(const std::string& assetPath)
{
	for (auto image : imageAssetArray)
	{
		if (image.second.assetPath == assetPath)
		{
			return image.first;
		}
	}

	const int32 newAssetId = NewAssetId();
	SImage newImage;
	SLoadImage(assetPath, newImage);
	imageAssetArray[newAssetId] = newImage;
	return newAssetId;
}

class Renderable_Image
{
public:
	int32 entityId;
	int32 assetId;

	Renderable_Image() = default;
	Renderable_Image(int32 inEntityId, int32 inAssetId) 
	{
		entityId = inEntityId;
		assetId = inAssetId;
	}

	void Render()
	{
		const Transform transform = transformArray[entityId];
		const Vector2D position = transform.Position; 
		const SImage image = imageAssetArray[assetId];
		const int32 width = image.GetHalfWidth();
		const int32 height = image.GetHalfHeight();
		DrawImage(image, Vector2D{position.x - width, position.y - height});	
	}
};

class Renderable_Square
{
public:
	int32 entityId;
	Color color;

	Renderable_Square() = default;

	void Render()
	{
		const Transform& transform = transformArray[entityId];
		DrawRectangle(transform.Position, transform.Scale, color);	
	}
};

class ImageRenderManager
{
public:
	void Update()
	{
		for (auto renderPair: renderableImagesArray)
		{
			renderPair.second.Render();
		}
	}
};

class SquareRenderManager
{
public:
	void Update()
	{
		for (auto renderPair : renderableSquareArray)
		{
			renderPair.second.Render();
		}
	}
};

void CreateBullet(const Transform& inTransform)
{
	const int32 entityId = NewId();
	transformArray[entityId] = Transform{ inTransform.Position, Vector2D {2.0f, 5.0f}};
	attributesArray[entityId] = Attributes{-100.f};
	renderableSquareArray[entityId] = Renderable_Square {entityId, White };
	bulletArray.push_back(entityId);
}

void DeleteBullet(int32 entityId)
{
	transformArray.erase(entityId);
	renderableImagesArray.erase(entityId);
	
	for (auto it = bulletArray.begin(); it != bulletArray.end();)
	{
		if (*it == entityId)
		{
			bulletArray.erase(it);
			break;
		}
	}
}

class PlayerControl
{
public:
	int32 entityId;
	
	void Update(float deltaTime)
	{
		Transform& transform = transformArray[entityId];
		if (IsKeyDown(ARROW_LEFT))
		{
			transform.Position.x -= attributesArray[entityId].SPEED * deltaTime;
		}
		if (IsKeyDown(ARROW_RIGHT))
		{
			transform.Position.x += attributesArray[entityId].SPEED * deltaTime;		
		}
		if (IsKeyDown(SPACEBAR) && bulletArray.size() <= 0)
		{
			CreateBullet(transform);			
		}
	}
};

class ControllerManager
{
public:
	void Update(float deltaTime)
	{
		for (auto controllerPair : playerControlArray)
		{
			controllerPair.second.Update(deltaTime);
		}
	}
};



class BulletManager
{
public:
	void Update(float deltaTime)
	{
		for (auto entityId : bulletArray)
		{
			Transform& transform = transformArray[entityId];
			Attributes& attribute = attributesArray[entityId];
			Vector2D& position = transform.Position;
			position.y += attribute.SPEED * deltaTime;

			if (position.y <= 0.f)
			{
				DeleteBullet(entityId);
			}
		}
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

std::vector<Invader*> Invaders;

ImageRenderManager renderManager;
SquareRenderManager squareRenderManager;
ControllerManager controllerManager;
BulletManager bulletManager;

// Space invader is using these window settings 
// static constexpr int32 Width = 160 * 2; // 160
// static constexpr int32 Height = 120 * 2; // 120
// static constexpr int32 PixelScale = 4;


void Start()
{
	// Creating new player
	{
		const int32 newId = NewId();
		transformArray[newId] = Transform{Vector2D{Cast<float>(Width / 2), Cast<float>(Height - 10)}};
		renderableImagesArray[newId] = Renderable_Image { newId, GetAssetId("Assets/SpaceInvader/Spaceship.png")};
		playerControlArray[newId] = PlayerControl{ newId };
		attributesArray[newId] = Attributes {100.f };
		playerEntityId = newId;
	}

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

void Tick(float deltaTime)
{
	Clear();
	RenderGrid();

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
		// SRect invaderRect = invader->GetCollisionRect();
		// SRect bulletRect = Bullet.GetCollisionRect();
		// if (invaderRect.IsRectangleOverlapping(bulletRect))
		// {
		// 	invader->Alive = false;
		// 	Bullet.Position = Vector2D{-100.f, -100.f};
		// 	continue;
		// }
		
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

	controllerManager.Update(deltaTime);
	bulletManager.Update(deltaTime);
	renderManager.Update();
	squareRenderManager.Update();
}