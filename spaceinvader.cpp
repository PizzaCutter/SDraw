#include "SEngine.h"
#include "SMath.h"

#include <iostream>

// Define an image
SSprite playerSprite;

float timer = 1.0f;
SRect destRect {0, 0, 8, 16};
SRect srcRect {0, 0, 8, 16};

void Start()
{
	// Load the image
	playerSprite = {};
	SLoadImage("Assets/SpaceInvader/Spaceship.png", playerSprite.srcImage);
	playerSprite.cellSizeX = 8;
}

void Tick(float deltaTime)
{
	Clear();

	playerSprite.UpdateSprite(deltaTime);
	DrawSprite(playerSprite);
}