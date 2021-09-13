#include <iostream>
#include <math.h>

static float worldTime = 0.0f;

static void InitializeGame()
{

}

static void UpdateGameAndRender(float deltaTime)
{

	
	worldTime += deltaTime;
	
	for (int i = 0; i < 600; ++i)
	{
		DrawRectangle(i, i);
	}

	int circlePosX = WindowData.width / 2;
	int circlePosY = WindowData.height / 2;
	int radius = WindowData.width / 4;
	float scaleSpeed = 0.1f;
	DrawCircleFilled(circlePosX, circlePosY, (abs(cos(worldTime * scaleSpeed)) * radius) + (radius / 4));
}