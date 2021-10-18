#include "SEngine.h"
#include "SMath.h"

#include <iostream>
#include <map>
#include <vector>

static constexpr float INVADER_SPEED = 50.f;
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

enum class Direction
{
	Left,
	Right,
	Up,
	Down
};

std::map<int32, Transform> transformArray;
std::map<int32, Attributes> attributesArray;

std::map<int32, class Renderable_Image> renderableImagesArray;
std::map<int32, class Renderable_Sprite> renderableSpriteArray;
std::map<int32, class Renderable_Square> renderableSquareArray;

std::map<int32, class PlayerControl> playerControlArray;
std::vector<int32> bulletArray;
std::vector<int32> invaderArray;

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

class Renderable_Sprite
{
public:
	int32 entityId;
	int32 assetId;
	
	Vector2D SpriteCellSize;

	float timePerFrame = 0.5f;

	Renderable_Sprite() = default;
	Renderable_Sprite(int32 inEntityId, int32 inAssetId, int32 inCellCountX, int32 inCellCountY = 1) 
	{
		entityId = inEntityId;
		assetId = inAssetId;
		cellCountX = inCellCountX;
		cellCountY = inCellCountY;

		const SImage& image = imageAssetArray[assetId];
		SpriteCellSize.x = image.width / cellCountX;
		SpriteCellSize.y = image.height / cellCountY;
	}

	void Render(float deltaTime)
	{
		const Transform transform = transformArray[entityId];
		const Vector2D position = transform.Position; 
		const SImage image = imageAssetArray[assetId];

		SRect srcRect = SRect {position.x, position.y, SpriteCellSize.x * transform.Scale.x, SpriteCellSize.y  * transform.Scale.y};
		SRect dstRect = SRect {SpriteCellSize.x * index, 0, SpriteCellSize.x, SpriteCellSize.y };
		DrawSprite(image, srcRect, dstRect);
		
		UpdateSprite(deltaTime);
	}

	void UpdateSprite(float deltaTime)
	{
		internalTimer += deltaTime;

		while(internalTimer >= timePerFrame)
		{
			index++;
			index %= cellCountX;
			internalTimer -= timePerFrame;
		}
	}

private:
	int32 index = 0;
	float internalTimer = 0.0f;
	int32 cellCountX = 1;
	int32 cellCountY = 1;
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
	void Update(float deltaTime)
	{
		for (auto renderPair: renderableImagesArray)
		{
			renderPair.second.Render();
		}
	}
};

class SpriteRenderManager
{
public:
	void Update(float deltaTime)
	{
		for (std::pair<const int32, Renderable_Sprite>& renderPair : renderableSpriteArray)
		{
			renderPair.second.Render(deltaTime);
		}
	}
};

class SquareRenderManager
{
public:
	void Update(float deltaTime)
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

class InvaderManager
{
public:
	Direction movementDirection = Direction::Right;
	
	void Update(float deltaTime)
	{
		// Update space invader positions
		for (auto entityId : invaderArray)
		{
			Transform& transform = transformArray[entityId];

			if (movementDirection == Direction::Left)
			{
				transform.Position.x -= INVADER_SPEED * deltaTime;	
			}else if (movementDirection == Direction::Right)
			{
				transform.Position.x += INVADER_SPEED * deltaTime;
			}
		}

		Direction prevMovementDirection = movementDirection;
		// Check if we should start moving to the other side of the screen
		for (auto entityId : invaderArray)
		{
			Transform& transform = transformArray[entityId];
			const Renderable_Sprite& sprite = renderableSpriteArray[entityId]; 
			
			if (movementDirection == Direction::Right)
			{
				if (transform.Position.x >= Width - sprite.SpriteCellSize.x)
				{
					movementDirection = Direction::Left;
					break;
				}
			}else if (movementDirection == Direction::Left)
			{
				if (transform.Position.x <= 0)
				{
					movementDirection = Direction::Right;
					break;
				}
			}	
		}

		// Move all space invaders down
		if (prevMovementDirection != movementDirection)
		{
			for (auto entityId : invaderArray)
			{
				Transform& transform = transformArray[entityId];
				transform.Position.y += 15.f; 
			}
		}
	}
};

ImageRenderManager renderManager;
SpriteRenderManager spriteRenderManager;
SquareRenderManager squareRenderManager;
ControllerManager controllerManager;
BulletManager bulletManager;
InvaderManager invaderManager;

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

	// Creating lot's of space invaders :) 
	const int32 invaderCountHorizontal = 10;
	const int32 invaderCountVertical = 3;
	for (int x = 0; x < invaderCountHorizontal; ++x)
	{
		for (int y = 0; y < invaderCountVertical; ++y)
		{
			const float xPos = x * INVADER_X_OFFSET;
			const float yPos = y * INVADER_Y_OFFSET;
			
			const int32 newId = NewId();
			transformArray[newId] = Transform{Vector2D{xPos, yPos}, Vector2D { 0.5f, 0.5f }};
			renderableSpriteArray[newId] = Renderable_Sprite { newId, GetAssetId("Assets/SpaceInvader/Invader_01.png"), 2, 1};
			invaderArray.push_back(newId);
		}
	}
}

void Tick(float deltaTime)
{
	Clear();
	RenderGrid();

	controllerManager.Update(deltaTime);
	invaderManager.Update(deltaTime);
	bulletManager.Update(deltaTime);
	
	renderManager.Update(deltaTime);
	spriteRenderManager.Update(deltaTime);
	squareRenderManager.Update(deltaTime);
}