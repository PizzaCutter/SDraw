#include <algorithm>

#include "SEngine.h"
#include "SMath.h"

#include <iostream>
#include <map>
#include <vector>

static constexpr float INVADER_SPEED = 5.f;
static constexpr float  INVADER_X_OFFSET = 15.f;
static constexpr float  INVADER_Y_OFFSET = 10.f;
static constexpr float INVADER_MOVE_STEP_TIME = 0.5f;
static constexpr float INVADER_SHOOT_CHANCE = 0.1f;

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
	int32 HEALTH;
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
std::map<int32, class CollisionBox> collisionBoxArray;

std::map<int32, class PlayerControl> playerControlArray;
std::map<int32, bool> bulletArray;
std::map<int32, bool> playerBulletArray;
std::map<int32, bool> enemyBulletArray;

std::map<int32, bool> invaderArray;

std::map<int32, SImage> imageAssetArray;

int32 playerEntityId;
int32 playerScore = 0;

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
		DrawFilledRectangle(transform.Position, transform.Scale, color);	
	}
};

class CollisionBox
{
public:
	int32 EntityId;
	Vector2D Scale;
	Vector2D Offset;

	SRect GetRect() const
	{
		const Transform& transform = transformArray[EntityId];
		return SRect { transform.Position.x + Offset.x, transform.Position.y + Offset.y, Scale.x, Scale.y };	
	}

	bool IsColliding(int32 inEntityId) const
	{
		const CollisionBox& collisionBox = collisionBoxArray[EntityId];
		const CollisionBox& otherCollisionBox = collisionBoxArray[inEntityId];
		return collisionBox.GetRect().IsRectangleOverlapping(otherCollisionBox.GetRect());
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

class CollisionRenderManager
{
public:
	void Update(float deltaTime)
	{
		for (std::pair<const int32, CollisionBox>& colliders : collisionBoxArray)
		{
			const Transform& transform = transformArray[colliders.first];
			DrawRectangle(Vector2D {transform.Position.x + colliders.second.Offset.x, transform.Position.y + colliders.second.Offset.y }, colliders.second.Scale, Green);
		}	
	}
};

int32 CreateBullet(const Transform& inTransform, float speed)
{
	const int32 entityId = NewId();
	transformArray[entityId] = Transform{ inTransform.Position, Vector2D {2.0f, 5.0f}};
	attributesArray[entityId] = Attributes{speed};
	renderableSquareArray[entityId] = Renderable_Square {entityId, White };
	collisionBoxArray[entityId] = CollisionBox { entityId, transformArray[entityId].Scale };
	bulletArray[entityId] = true;
	return entityId;
}

void DeleteBullet(int32 entityId)
{
	transformArray.erase(entityId);
	attributesArray.erase(entityId);
	renderableSquareArray.erase(entityId);
	collisionBoxArray.erase(entityId);
	bulletArray.erase(entityId);
	playerBulletArray.erase(entityId);
	enemyBulletArray.erase(entityId);
}

void RemovePlayerHealth()
{
	Attributes& attribute = attributesArray[playerEntityId];
	attribute.HEALTH = std::clamp(attribute.HEALTH - 1, 0, attribute.HEALTH);
}

void DeleteSpaceInvader(int32 entityId);

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
		if (IsKeyDown(SPACEBAR) && playerBulletArray.size() <= 0)
		{
			int32 entityId = CreateBullet(transform, -100.f);
			playerBulletArray[entityId] = true;
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
		std::vector<int32> bulletsToDelete;
		for (auto entityId : bulletArray)
		{
			Transform& transform = transformArray[entityId.first];
			Attributes& attribute = attributesArray[entityId.first];
			Vector2D& position = transform.Position;
			position.y += attribute.SPEED * deltaTime;

			if (position.y <= 0.f)
			{
				bulletsToDelete.push_back(entityId.first);
				continue;
			}
			if (position.y >= Height)
			{
				bulletsToDelete.push_back(entityId.first);
				continue;
			}
		}

		for (auto bulletEntityId : bulletsToDelete)
		{
			DeleteBullet(bulletEntityId);	
		}
	}
};

class PlayerBulletManager
{
public:
	void Update(float deltaTime)
	{
		int32 bulletToDelete = -1;
		int32 invaderToDelete = -1;
		
		for (auto bulletEntityId : playerBulletArray)
		{
			const CollisionBox& bulletCollider = collisionBoxArray[bulletEntityId.first]; 
			for (const auto invaderEntityId : invaderArray)
			{
				if (bulletCollider.IsColliding(invaderEntityId.first))
				{
					bulletToDelete = bulletEntityId.first;
					invaderToDelete = invaderEntityId.first;
				}
			}
		}

		if (bulletToDelete != -1)
		{
			DeleteBullet(bulletToDelete);
		}
		if (invaderToDelete != -1)
		{
			DeleteSpaceInvader(invaderToDelete);
			playerScore += 25;
		}
	}
};


class InvaderBulletManager
{
public:
	void Update(float deltaTime)
	{
		std::vector<int32> bulletToDelete;

		for (auto bulletEntityId : enemyBulletArray)
		{
			const CollisionBox& playerCollider = collisionBoxArray[playerEntityId];
			if (playerCollider.IsColliding(bulletEntityId.first))
			{
				bulletToDelete.push_back(bulletEntityId.first);		
			}
		}

		for (int32 bulletEntityId : bulletToDelete)
		{
			DeleteBullet(bulletEntityId);
			RemovePlayerHealth();
		}
	}
};

class InvaderManager
{
public:
	Direction movementDirection = Direction::Right;
	float timer = 0.0f;
	
	void Update(float deltaTime)
	{
		UpdateMovement();
		
		timer += deltaTime;
		
		const float normalizedRandom = (Cast<float>(std::rand()) / Cast<float>(RAND_MAX)) * 100.f;
		int32 invaderIndexToShoot = (std::rand() / (RAND_MAX / (invaderArray.size() - 1)));
		invaderIndexToShoot = normalizedRandom < INVADER_SHOOT_CHANCE ? invaderIndexToShoot : -1;

		if (invaderIndexToShoot != -1)
		{
			std::vector<int> keys;
			for (auto invader : invaderArray)
			{
				keys.push_back(invader.first);	
			}

			const int32 invaderEntityId = keys[invaderIndexToShoot];
			const Transform& transform = transformArray[invaderEntityId];
			const int32 bulletEntityId = CreateBullet(transform, 100.f);
			enemyBulletArray[bulletEntityId] = true;	
		}
	}

	void UpdateMovement()
	{
		while(timer >= INVADER_MOVE_STEP_TIME)
		{
			// Update space invader positions
			for (auto entityId : invaderArray)
			{
				Transform& transform = transformArray[entityId.first];

				if (movementDirection == Direction::Left)
				{
					transform.Position.x -= INVADER_SPEED;
				}
				else if (movementDirection == Direction::Right)
				{
					transform.Position.x += INVADER_SPEED;
				}
			}

			Direction prevMovementDirection = movementDirection;
			// Check if we should start moving to the other side of the screen
			for (auto entityId : invaderArray)
			{
				Transform& transform = transformArray[entityId.first];
				const Renderable_Sprite& sprite = renderableSpriteArray[entityId.first];

				if (movementDirection == Direction::Right)
				{
					if (transform.Position.x >= Width - sprite.SpriteCellSize.x)
					{
						movementDirection = Direction::Left;
						break;
					}
				}
				else if (movementDirection == Direction::Left)
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
					Transform& transform = transformArray[entityId.first];
					transform.Position.y += 15.f;
				}
			}
			timer -= INVADER_MOVE_STEP_TIME;
		}
	}
};

ImageRenderManager renderManager;
SpriteRenderManager spriteRenderManager;
SquareRenderManager squareRenderManager;

ControllerManager controllerManager;
BulletManager bulletManager;
InvaderManager invaderManager;
PlayerBulletManager playerBulletManager;
InvaderBulletManager invaderBulletManager;

CollisionRenderManager debugCollisionRenderManager;

void CreateSpaceInvader(const Vector2D& inPos)
{
	const int32 newId = NewId();
	transformArray[newId] = Transform{inPos , Vector2D { 0.5f, 0.5f }};
			
	const int32 assetId = GetAssetId("Assets/SpaceInvader/Invader_01.png");
	const Renderable_Sprite sprite = Renderable_Sprite { newId, assetId, 2, 1};
	renderableSpriteArray[newId] = sprite;
			
	collisionBoxArray[newId] = CollisionBox { newId,  { sprite.SpriteCellSize.x * transformArray[newId].Scale.x , sprite.SpriteCellSize.y * transformArray[newId].Scale.y } };
	invaderArray[newId] = true;	
}

void DeleteSpaceInvader(int32 entityId)
{
	transformArray.erase(entityId);
	renderableSpriteArray.erase(entityId);
	collisionBoxArray.erase(entityId);
	invaderArray.erase(entityId);
}

void Start()
{
	srand (static_cast <unsigned> (time(0)));
	
	// Creating new player
	{
		const int32 newId = NewId();
		transformArray[newId] = Transform{Vector2D{Cast<float>(Width / 2), Cast<float>(Height - 10)}};
		const Vector2D& scale = transformArray[newId].Scale;
		const int32 assetId =GetAssetId("Assets/SpaceInvader/Spaceship.png");
		renderableImagesArray[newId] = Renderable_Image { newId, assetId};
		const SImage& image = imageAssetArray[assetId];
		playerControlArray[newId] = PlayerControl{ newId };
		attributesArray[newId] = Attributes {100.f, 3 };
		
		Vector2D collisionScale { image.width * transformArray[newId].Scale.x, image.height * transformArray[newId].Scale.y * 0.5f };
		Vector2D offset {-image.width * 0.5f, 0.f };
		collisionBoxArray[newId] = CollisionBox { newId, collisionScale, offset };
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
			const float yPos = y * INVADER_Y_OFFSET + 25.f;
			CreateSpaceInvader(Vector2D{xPos, yPos});	
		}
	}
}

void RenderUI()
{
	Attributes& attributes = attributesArray[playerEntityId];
	const std::string score = "SCORE " + std::to_string(playerScore);
	DrawString(Vector2D{5.f, 5.f }, score, Alignment::Left, White, 2);
	const std::string lives = "LIVES " + std::to_string(attributes.HEALTH); 
	DrawString(Vector2D{GetHalfWidth(), 5.f }, lives, Alignment::Left, White, 2);	
}

void Tick(float deltaTime)
{
	Clear();
	RenderGrid();

	controllerManager.Update(deltaTime);
	invaderManager.Update(deltaTime);
	bulletManager.Update(deltaTime);
	playerBulletManager.Update(deltaTime);
	invaderBulletManager.Update(deltaTime);
	
	renderManager.Update(deltaTime);
	spriteRenderManager.Update(deltaTime);
	squareRenderManager.Update(deltaTime);

	debugCollisionRenderManager.Update(deltaTime);

	RenderUI();
}