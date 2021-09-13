#include "SDL_Renderer.h"
#include <SDL2/SDL.h>
#include <iostream>

void SetColor(Color newColor)
{
	unsigned int r = (newColor >> 16) & 0xFF;
	unsigned int g = (newColor >> 8) & 0xFF;
	unsigned int b = (newColor >> 0) & 0xFF; 
	unsigned int a = (newColor >> 24) & 0xFF;
	
	SDL_SetRenderDrawColor(RenderData.renderer, r, g, b, a);
}

void DrawPoint(int posX, int posY, Color color)
{
	SetColor(color);
	SDL_RenderDrawPoint(RenderData.renderer, posX, posY);	
}

void DrawLine(int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
	SetColor(color);
	SDL_RenderDrawLine(RenderData.renderer, startPosX, startPosY, endPosX, endPosY);
}

void DrawRectangle(int posX, int posY, Color color)
{
	SetColor(color);
	DrawPoint(posX, posY, color);
}

void DrawCircle(int x, int y, int radius, Color color)
{
	SetColor(color);
    int offsetx, offsety, d;

    offsetx = 0;
    offsety = radius;
    d = radius -1;

    while (offsety >= offsetx) {
        DrawPoint(x + offsetx, y + offsety, color);
        DrawPoint(x + offsety, y + offsetx, color);
        DrawPoint(x - offsetx, y + offsety, color);
        DrawPoint(x - offsety, y + offsetx, color);
        DrawPoint(x + offsetx, y - offsety, color);
        DrawPoint(x + offsety, y - offsetx, color);
        DrawPoint(x - offsetx, y - offsety, color);
        DrawPoint(x - offsety, y - offsetx, color);

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}


void DrawCircleFilled(int x, int y, int radius, Color color)
{
	SetColor(color);

    int offsetx, offsety, d;

    offsetx = 0;
    offsety = radius;
    d = radius -1;

    while (offsety >= offsetx) {

        DrawLine(x - offsety, y + offsetx,
                                     x + offsety, y + offsetx, color);
        DrawLine(x - offsetx, y + offsety,
                                     x + offsetx, y + offsety, color);
        DrawLine(x - offsetx, y - offsety,
                                     x + offsetx, y - offsety, color);
        DrawLine(x - offsety, y - offsetx,
                                     x + offsety, y - offsetx, color);

        if (d >= 2*offsetx) {
            d -= 2*offsetx + 1;
            offsetx +=1;
        }
        else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        }
        else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}

int GameEngine::Start()
{
	RenderData = {};
	WindowData = {};

	WindowData.width = 600;	
	WindowData.height = 600;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
		std::cout << "Initialization failed" << std::endl;
	}

	RenderData.window = SDL_CreateWindow("Practice making sdl Window",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WindowData.width,
			WindowData.height, SDL_WINDOW_SHOWN);

	if (RenderData.window == NULL) {
		SDL_Quit();
		return 2;
	}

	// We create a renderer with hardware acceleration, we also present according with the vertical sync refresh.
	RenderData.renderer = SDL_CreateRenderer(RenderData.window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) ;

	bool quit = false;
	SDL_Event event;

	Initialize();

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = true;
			}
		}

		// We clear what we draw before
		SDL_RenderClear(RenderData.renderer);
		// Set our color for the draw functions
		SDL_SetRenderDrawColor(RenderData.renderer, 0xFF, 0xFF, 0xFF, 0xFF);

		// Now we can draw our point
		float deltaTime = 0.16;
		UpdateGame(deltaTime);
		
		// Set the color to what was before
		SDL_SetRenderDrawColor(RenderData.renderer, 0x00, 0x00, 0x00, 0xFF);
		// .. you could do some other drawing here
		// And now we present everything we draw after the clear.
		SDL_RenderPresent(RenderData.renderer);
	}

	SDL_DestroyWindow(RenderData.window);
	// We have to destroy the renderer, same as with the window.
	SDL_DestroyRenderer(RenderData.renderer);
	SDL_Quit();

	return EXIT_SUCCESS;
}