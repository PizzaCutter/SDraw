#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

static constexpr int Width = 160;
static constexpr int Height = 120;
static constexpr int PixelScale = 5;

// Attempt at building small game/rendering framework based on immediate2D https://github.com/npiegdon/immediate2d
using Color = unsigned int;

inline Color MakeColor(int r, int g, int b)
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
void SetPixel(int x, int y, Color c);
void DrawRectangle(int x, int y, int width, int height, Color c);

