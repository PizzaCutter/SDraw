// TODO[rsmekens]: compiler flag for macos
//#include "SDL_Renderer.h"
#include "SPixelGameEngine.h"

#include <iostream>
#include <math.h>


// class Graphing : public PixelGameEngine
// {
// public:
// 	float worldTime = 0.0f;
//
// 	Graphing() 
// 	{
// 	}
//
// 	double f(double x)
// 	{
// 		return sin(x);
// 	}
//
// 	virtual void Initialize() override
// 	{
//
// 	}
//
// 	virtual void UpdateGame(float deltaTime) override
// 	{
// 	worldTime += deltaTime;
// 	
// 	int Width = WindowData.width;
// 	int Height = WindowData.height;
// 	
// 	// Draw our axes
// 	DrawLine(0, Height / 2, Width, Height / 2, LightGray);
// 	DrawLine(Width / 2, 0, Width / 2, Height, LightGray);
//
// 	for (int p = 0; p < Width; ++p)
// 	{
// 		// We'll need to transform from our pixel coordinate system to the usual Cartesian coordinates
// 		double x = p;
//
// 		// First, shift x=0 to the center of the screen (instead of the left edge)
// 		x -= Width / 2;
//
// 		// Scale x down, effectively setting our graph's "Window" to something like w=(-8, 8), h=(-6, 6)
// 		x /= 10.0;
//
// 		x -= worldTime;
//
// 		// Run the function!
// 		double y = f(x);
//
// 		// Now we have to scale our result back up
// 		y *= 100;
//
// 		// In computer graphics, y increases downward
// 		// In Cartesian coordinates, y increases upward
// 		y = -y;
//
// 		// Finally, shift it to the center of the screen (instead of the top edge)
// 		y += Height / 2;
//
// 		DrawPoint(p, (int)y, LightRed);
// 	}
// 	}
// };

int main()
{
	std::cout << "Main" << std::endl;
	return 0;
	//Graphing app;
	//return app.Start();
}