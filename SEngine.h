#pragma once

#include "SMath.h"
#include "Typedefs.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <string>

static constexpr int32 Width = 160; // 160
static constexpr int32 Height = 120; // 120
static constexpr int32 PixelScale = 4;

// Attempt at building small game/rendering framework based on immediate2D https://github.com/npiegdon/immediate2d
using Color = unsigned int;

inline Color MakeColor(int32 r, int32 g, int32 b)
{
	return (0xFF << 24) // pack alpha in last 8 bits but just ignore for now
			| ((r & 0xFF) << 16) // pack red value in 16-24 bits
			| ((g & 0xFF) << 8) // Pack green values in 8-16 bits
			| ((b & 0xFF) << 0); // Pack blue values in first 8 bits
}

// Here are some colors (from the old, 16-color EGA palette) to get you
// started.  You can make your own using the same MakeColor function.
static const Color Black =        MakeColor(  0,   0,   0);
static const Color Blue =         MakeColor(  0,   0, 170);
static const Color Green =        MakeColor(  0, 170,   0);
static const Color Cyan =         MakeColor(  0, 170, 170);
static const Color Red =          MakeColor(170,   0,   0);
static const Color Magenta =      MakeColor(170,   0, 170);
static const Color Brown =        MakeColor(170,  85,   0);
static const Color LightGray =    MakeColor(170, 170, 170);
static const Color DarkGray =     MakeColor( 85,  85,  85);
static const Color LightBlue =    MakeColor( 85,  85, 170);
static const Color LightGreen =   MakeColor( 85, 255,  85);
static const Color LightCyan =    MakeColor( 85, 255, 255);
static const Color LightRed =     MakeColor(255,  85,  85);
static const Color LightMagenta = MakeColor(255,  85, 255);
static const Color Yellow =       MakeColor(255, 255,  85);
static const Color White =        MakeColor(255, 255, 255);

void Clear(Color clearColor = Black);
void RenderGrid();
void SetPixel(Vector2D pos, Color c);
void SetPixel(int32 x, int32 y, Color c);
void DrawRectangle(Vector2D pos, Vector2D size, Color c);
void DrawRectangle(int32 x, int32 y, int32 width, int32 height, Color c);
void DrawLine(int32 startX, int32 startY, int32 endX, int32 endY, Color c);

// INPUT
bool IsKeyDown(char key);
int32 GetMouseX();
int32 GetMouseY();
// ~INPUT

// APPLICATION
void SetApplicationName(const std::string& newApplicationName);
// ~APPLICATION

// RENDERING UI 
enum Alignment { Left, Right, Center };
int32 GetStringWidth(const std::string& s);
void DrawString(Vector2D pos, const std::string& s, Alignment alignment, Color color, int32 size);
void DrawString(int32 x, int32 y, const std::string& s, Alignment alignment, Color color, int32 size);
// ~RENDERING UI


