#include "SEngine.h"

#include <windowsx.h>

// INCLUDES FOR GDIPLUS
#include <objidl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
// ~INCLUDES FOR GDIPLUS

// INCLUDES FOR MIDI OUTPUT
#include <mmeapi.h>
#pragma comment(lib, "winmm.lib")
// ~INCLUDES FOR MIDI OUTPUT

#include <algorithm>
#include <iostream>
#include <thread>
#include <chrono>
#include <deque>
#include <iomanip>
#include <sstream>
#include <vector>

using namespace std;
using namespace std::chrono;

#define MAX_LOADSTRING 100
#define StringToCString(path) StringToWString(path).c_str()

void Start();
void Tick(float deltaTime);

static unique_ptr<Gdiplus::Bitmap> bitmap;
static unique_ptr<Gdiplus::Graphics> graphics;
static unique_ptr<Gdiplus::Bitmap> bitmapOther;
static unique_ptr<Gdiplus::Graphics> graphicsOther;
static std::string applicationName = "SDraw Application";

struct MusicNote { uint8_t noteId; std::chrono::milliseconds duration; };
static deque<MusicNote> musicQueue;

static unique_ptr<std::thread> musicThread;

static bool bLockFrameRate = false;
static uint32 maxFrameRate = 120;

// TODO[rsmekens]: figure a way to create a better way to map this so we aren't reliant on win32 values
static vector<char> inputBuffer;
static vector<char> keysDown;
int mouseX {-1}, mouseY {-1};

static bool temp = false;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void Clear(Color c)
{
	if (graphics == nullptr)
	{
		return;
	}
	graphics->Clear(Gdiplus::Color(c));
}

void RenderGrid()
{
	DrawLine(Width / 2, 0, Width / 2, Height, Red);
	DrawLine(0, Height / 2, Width, Height / 2, Green);
}

void SetPixel(Vector2D pos, Color c)
{
	SetPixel(static_cast<int32>(std::round(pos.x)), static_cast<int32>(std::round(pos.y)), c);		
}

void SetPixel(int x, int y, Color c)
{
	Gdiplus::SolidBrush solidBrush(c);
	graphics->FillRectangle(&solidBrush, x, y, 1, 1);
}

void DrawRectangle(Vector2D pos, Vector2D size, Color c)
{
	DrawRectangle(static_cast<int32>(std::round(pos.x)), static_cast<int32>(std::round(pos.y)), static_cast<int32>(std::round(size.x)), static_cast<int32>(std::round(size.y)), c);	
}

void DrawRectangle(int32 x, int32 y, int32 width, int32 height, Color c)
{
	Gdiplus::SolidBrush solidBrush(c);
	graphics->FillRectangle(&solidBrush, x, y, width, height);
}

void DrawLine(int32 startX, int32 startY, int32 endX, int32 endY, Color c)
{
	Gdiplus::Pen pen(c);
	graphics->DrawLine(&pen, startX, startY, endX, endY);
}

void DrawImage(const SImage& image, Vector2D position)
{
	DrawImage(image, Cast<int32>(position.x), Cast<int32>(position.y));	
}

void DrawImage(const SImage& image, int32 startX, int32 startY, int32 width, int32 height)
{
	const int32 drawWidth = width == -1 ? image.width : width;
	const int32 drawHeight = height == -1 ? image.height : height;
	graphics->DrawImage((Gdiplus::Bitmap*)image.bitmap, startX, startY, drawWidth, drawHeight);	
}

void DrawImage(const SImage& image, const SRect& inDestRect, const SRect& inSrcRect)
{
	Gdiplus::RectF destRect { inDestRect.x, inDestRect.y, inDestRect.width, inDestRect.height };
	Gdiplus::RectF srcRect { inSrcRect.x, inSrcRect.y, inSrcRect.width, inSrcRect.height };
	graphics->DrawImage((Gdiplus::Bitmap*)image.bitmap, destRect, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, Gdiplus::UnitPixel);
}

void DrawSprite(const SSprite& sprite, const Vector2D& position)
{
	Gdiplus::RectF destRect { position.x, position.y, sprite.cellSizeX, static_cast<float>(sprite.srcImage.height)};
	Gdiplus::RectF srcRect { sprite.cellSizeX * sprite.index, 0, sprite.cellSizeX, static_cast<float>(sprite.srcImage.height) };
	graphics->DrawImage((Gdiplus::Bitmap*)sprite.srcImage.bitmap, destRect, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, Gdiplus::UnitPixel);	
}

void DrawSprite(const SSprite& sprite, const Vector2D& position, const Vector2D& scale)
{
	Gdiplus::RectF destRect { position.x, position.y, sprite.cellSizeX * scale.x, static_cast<float>(sprite.srcImage.height * scale.y)};
	Gdiplus::RectF srcRect { sprite.cellSizeX * sprite.index, 0, sprite.cellSizeX, static_cast<float>(sprite.srcImage.height) };
	graphics->DrawImage((Gdiplus::Bitmap*)sprite.srcImage.bitmap, destRect, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, Gdiplus::UnitPixel);	
}

void DrawSprite(const SImage& image, const SRect& inDestRect, const SRect& inSrcRect)
{
	Gdiplus::RectF destRect { inDestRect.x, inDestRect.y, inDestRect.width, inDestRect.height };
	Gdiplus::RectF srcRect { inSrcRect.x, inSrcRect.y, inSrcRect.width, inSrcRect.height };
	graphics->DrawImage((Gdiplus::Bitmap*)image.bitmap, destRect, srcRect.X, srcRect.Y, srcRect.Width, srcRect.Height, Gdiplus::UnitPixel);		
}

bool IsKeyDown(char key)
{
	for (const char& curKey : keysDown)
	{
		if(key == curKey)
		{
			return true;
		}
	}
	return false;
}

int GetMouseX()
{
	return mouseX;
}

int GetMouseY()
{
	return mouseY;
}

void SetApplicationName(const std::string& newApplicationName)
{
	applicationName = newApplicationName;
}

void AddKeyDown(char key)
{
	if (!IsKeyDown(key))
	{
		keysDown.push_back(key);
	}
}

void RemoveKeyDown(char key)
{
	for (int i = 0; i < keysDown.size(); i++)
	{
		if(key == keysDown[i])
		{
			keysDown.erase(keysDown.begin() + i);
			return;
		}
	}
}

void PlayMidiNote(int noteId, int ms)
{
	if (noteId < 0)
	{
		return;
	}
	
	if (ms < 0)
	{
		return;
	}
	
	// TODO[rsmekens]: figure out what this noteId conversion does exactly
	musicQueue.push_back(MusicNote{uint8(uint8(noteId) & 0x7F), milliseconds(ms)});
}

void MusicTick()
{
	HMIDIOUT synth = nullptr;
	if (midiOutOpen(&synth, MIDI_MAPPER, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR )
	{
		return;
	}

	// We always use the "Lead 1 (Square)" instrument because it sounds like the PC speaker
	constexpr uint8_t Instrument = 80;
	midiOutShortMsg(synth, 0xC0 | (Instrument << 8));
	
	while(true)
	{
		MusicNote n;
		{
			if (musicQueue.empty())
			{
				this_thread::sleep_for(1ms);
				continue;
			}
			
			n = musicQueue.front();
			musicQueue.pop_front();
		}

		if (n.noteId != 0)
		{
			midiOutShortMsg(synth, 0x00700090 | (n.noteId << 8));
		}
		this_thread::sleep_for(n.duration);
		if (n.noteId != 0)
		{
			midiOutShortMsg(synth, 0x00000090 | (n.noteId << 8));
		}
	}

	midiOutClose(synth);
}


int APIENTRY wWinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPWSTR    lpCmdLine,
					  int nCmdShow)
{
	WNDCLASS windowClass = {}; // reserves memory on the stack but set's everything to zero
	// https://docs.microsoft.com/en-us/windows/win32/winmsg/window-class-styles
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	// Flags that decide that we need to repaint the whole window  (otherwise will only repaint the resize section)
	windowClass.lpfnWndProc = WndProc;
	windowClass.hInstance = hInstance;
	//windowClass.hIcon;
	windowClass.lpszClassName = L"PONG";
	if (!RegisterClass(&windowClass)) return 1;

	const DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	
	RECT r{ 0, 0, Width * PixelScale, Height * PixelScale};
	AdjustWindowRect(&r, style, FALSE);
	
	//HWND wnd = CreateWindowEx(0, wdc, style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);
	HWND window = CreateWindowEx(0, windowClass.lpszClassName, L"Window Name - SDRAW",
									WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
									r.right - r.left, r.bottom - r.top, 0, 0, hInstance, 0);
	if (window == nullptr) return 1;

	// Initialize GDI+.
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bitmap = make_unique<Gdiplus::Bitmap>(Width, Height);
	graphics = make_unique<Gdiplus::Graphics>(bitmap.get());
	bitmapOther = make_unique<Gdiplus::Bitmap>(Width, Height);
	graphicsOther = make_unique<Gdiplus::Graphics>(bitmapOther.get());

	Clear(Blue);
	// ~Initialize GDI+.

	// START MUSIC FUNCTIONALITY
	musicThread = make_unique<std::thread>(MusicTick);
	musicThread->detach();
	// ~START MUSIC FUNCTIONALITY

	ShowWindow(window, nCmdShow);
	UpdateWindow(window);

	Start();

	auto lastDraw = high_resolution_clock::now();
	MSG message;
	while (true)
	{
		if (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == WM_QUIT) break;
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		auto now = high_resolution_clock::now();
		duration<float, std::milli> f_millis = now - lastDraw; 
		auto f_secs = std::chrono::duration_cast<duration<float>>(f_millis);
		auto fps = (1.0f / (float)f_millis.count()) * 1000.0f;

		
		// std::cout << "Milli: " << i_millis.count() << '\n';
		// std::cout << "Sec: " << f_secs.count() << '\n';
		// std::cout << "FPS: " << fps << '\n';

		float targetMs = (1.0f / static_cast<float>(maxFrameRate) * 1000);
		if (bLockFrameRate && f_millis < duration<double, milli>(targetMs))
		{
			continue;	
		}else
		{
			std::stringstream stream;
			stream << applicationName;
			stream << std::fixed << std::setprecision(3) << " | Ms: " << f_millis.count();
			stream << std::fixed << std::setprecision(2) << " - FPS: " << fps;
			stream << std::fixed << std::setprecision(3) << " - Delta: " << f_secs.count();
			const std::string appName = stream.str(); 
			SetWindowText(window, StringToCString(appName));

			temp = !temp;
			Tick(f_secs.count());
			InvalidateRect(window, nullptr, false);
	
			inputBuffer.clear();

			lastDraw = now;	
		}
	}
	
	bitmap.reset();
	graphics.reset();
	bitmapOther.reset();
	graphicsOther.reset();
	Gdiplus::GdiplusShutdown(gdiplusToken);
	
	return (int) message.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC bitmapDC{};
	static HBITMAP hbitmap{};
	
	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);

			if (!hbitmap)
			{
				bitmapDC = CreateCompatibleDC(hdc);
				hbitmap = CreateCompatibleBitmap(hdc, Width, Height);
			}

			HANDLE old = SelectObject(bitmapDC, hbitmap);
			{
				Gdiplus::Graphics hdcG(bitmapDC);
				hdcG.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
				hdcG.DrawImage(bitmap.get(), 0, 0, 0, 0, Width, Height, Gdiplus::UnitPixel);
			}

			StretchBlt(hdc, 0, 0, Width * PixelScale, Height * PixelScale, bitmapDC, 0, 0, Width, Height, SRCCOPY);
			SelectObject(bitmapDC, old);

			EndPaint(hWnd, &ps);
		}
		break;
	case WM_KEYDOWN:
		AddKeyDown(static_cast<char>(wParam));
		if (wParam == VK_UP)
		{
			//std::cout << "Pressed UP arrow key" << std::endl;	
		}
		
		if (wParam == VK_ESCAPE)
		{
			// TODO[rsmekens]: quit application
		}
		break;
	case WM_MOUSEMOVE:
		mouseX = GET_X_LPARAM(lParam) / PixelScale;
		mouseY = GET_Y_LPARAM(lParam) / PixelScale;
		break;	
	case WM_KEYUP:
		RemoveKeyDown(static_cast<char>(wParam));	
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// This block of numbers encodes a monochrome, 5-pixel-tall font for the first 127 ASCII characters!
//
// Bits are shifted out one at a time as each row is drawn (top to bottom).  Because each glyph fits
// inside an at-most 5x5 box, we can store all 5x5 = 25-bits fit inside a 32-bit unsigned int with
// room to spare.  That extra space is used to store that glyph's width in the most significant nibble.
//
// The first 32 entries are unprintable characters, so each is totally blank with a width of 0
//
static const unsigned int Font[128] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0x10000000, 0x10000017, 0x30000C03, 0x50AFABEA, 0x509AFEB2, 0x30004C99, 0x400A26AA, 0x10000003, 0x2000022E, 0x200001D1, 0x30001445, 0x300011C4, 0x10000018, 0x30001084, 0x10000010, 0x30000C98,
	0x30003A2E, 0x300043F2, 0x30004AB9, 0x30006EB1, 0x30007C87, 0x300026B7, 0x300076BF, 0x30007C21, 0x30006EBB, 0x30007EB7, 0x1000000A, 0x1000001A, 0x30004544, 0x4005294A, 0x30001151, 0x30000AA1,
	0x506ADE2E, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
	0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x2000023F, 0x30006083, 0x200003F1, 0x30000822, 0x30004210,
	0x20000041, 0x300078BE, 0x30002ABF, 0x3000462E, 0x30003A3F, 0x300046BF, 0x300004BF, 0x3000662E, 0x30007C9F, 0x1000001F, 0x30003E08, 0x30006C9F, 0x3000421F, 0x51F1105F, 0x51F4105F, 0x4007462E,
	0x300008BF, 0x400F662E, 0x300068BF, 0x300026B2, 0x300007E1, 0x30007E1F, 0x30003E0F, 0x50F8320F, 0x30006C9B, 0x30000F83, 0x30004EB9, 0x30004764, 0x1000001F, 0x30001371, 0x50441044, 0x00000000,
};

int32 DrawCharacter(int32 left, int32 top, char charToDraw, Color color, int32 size)
{
	unsigned int glyph = Font[charToDraw];
	int width = glyph >> 28;

	int tempX = 0;
	for (int x = left; x < left + width; x++)
	{
		int tempY = 0;
		for (int y = top; y < top + 5; y++)
		{
			if ((glyph & 1) == 1)
			{
				if (size == 1)
				{
					SetPixel(x, y, color);
				}
				else
				{
					DrawRectangle(left + (tempX * size), top + (tempY * size), size, size, color);
				}
			}

			glyph = glyph >> 1;
			tempY++;
		}
		tempX++;
	}
	
	return width;
}

int32 GetStringWidth(const std::string& s)
{
	int32 width = 0;
	for (char c : s)
	{
		unsigned int glyph = Font[c];
		width += glyph >> 28;
	}
	return width;	
}

void DrawString(Vector2D pos, const std::string& s, Alignment alignment, Color color, int32 size)
{
	DrawString(static_cast<int32>(std::round(pos.x)), static_cast<int32>(std::round(pos.y)), s, alignment, color, size);
}

void DrawString(int32 x, int32 y, const std::string& s, Alignment alignment, const Color color, int32 size)
{
	const float stringWidthFloat = GetStringWidth(s) * size + ((s.size() - 1) * 0.5f) * size;
	const int32 stringWidth = static_cast<int32>(std::round(stringWidthFloat));
	
	switch (alignment) {
		case Right:
			x -= stringWidth;
			break;
		case Center:
			x -= static_cast<int32>(std::round(stringWidth / 2.0f));
			break;
	}

	int charIndex = 0;
	for (char c : s)
	{
		int32 characterWidth = DrawCharacter(x, y, c, color, size);
		x += (characterWidth * size) + static_cast<int32>(size * 0.5f);
		charIndex++;
	}
}


bool SLoadImage(const std::string& path, SImage& outImage)
{
	outImage = {};
	outImage.assetPath = path;
	Gdiplus::Bitmap* loadedBitmap = new Gdiplus::Bitmap(StringToCString(path));
	outImage.bitmap = loadedBitmap;
	outImage.width = loadedBitmap->GetWidth();
	outImage.height = loadedBitmap->GetHeight();
	return true;
}


