#include "SEngine.h"

// INCLUDES FOR GDIPLUS
#include <objidl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
// ~INCLUDES FOR GDIPLUS

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include <windowsx.h>

using namespace std;
using namespace std::chrono;

#define MAX_LOADSTRING 100

void Start();
void UpdateGame(float deltaTime);

static unique_ptr<Gdiplus::Bitmap> bitmap;
static unique_ptr<Gdiplus::Graphics> graphics;
static unique_ptr<Gdiplus::Bitmap> bitmapOther;
static unique_ptr<Gdiplus::Graphics> graphicsOther;

static vector<char> inputBuffer;
static vector<char> keysDown;
int mouseX {-1}, mouseY {-1};

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

void SetPixel(float x, float y, Color c)
{
	SetPixel(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)), c);		
}

void SetPixel(int x, int y, Color c)
{
	Gdiplus::SolidBrush solidBrush(c);
	graphics->FillRectangle(&solidBrush, x, y, 1, 1);
}

void DrawRectangle(float x, float y, float width, float height, Color c)
{
	DrawRectangle(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)), static_cast<int>(std::round(width)), static_cast<int>(std::round(height)), c);	
}

void DrawRectangle(int x, int y, int width, int height, Color c)
{
	Gdiplus::SolidBrush solidBrush(c);
	graphics->FillRectangle(&solidBrush, x, y, width, height);
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
	windowClass.lpszClassName = L"HandmadeHeroWindowClass";
	if (!RegisterClass(&windowClass)) return 1;

	const DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	
	RECT r{ 0, 0, Width * PixelScale, Height * PixelScale};
	AdjustWindowRect(&r, style, FALSE);
	
	//HWND wnd = CreateWindowEx(0, wdc, style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);
	HWND window = CreateWindowEx(0, windowClass.lpszClassName, L"Handmade Hero",
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
		auto duration = now - lastDraw;
		auto i_millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		auto f_secs = std::chrono::duration_cast<std::chrono::duration<float>>(duration);
		auto fps = (1.0f / (float)i_millis.count()) * 1000.0f;
		
		// std::cout << "Milli: " << i_millis.count() << '\n';
		// std::cout << "Sec: " << f_secs.count() << '\n';
		// std::cout << "FPS: " << fps << '\n';
		
		if (duration > 16ms)
		{
			UpdateGame(f_secs.count());
			InvalidateRect(window, nullptr, false);
			
			inputBuffer.clear();

			lastDraw = now;
		}//else
		{
			//std::this_thread::sleep_for(1ms);
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

int DrawCharacter(int left, int top, char charToDraw, Color color, int charIndex, int size)
{
	unsigned int glyph = Font[charToDraw];
	int width = glyph >> 28;

	for (int x = left; x < left + width; x++)
	{
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
					DrawRectangle(x * size, y * size, size, size, color);
				}
			}

			glyph = glyph >> 1;
		}
	}
	
	return width;
}

void DrawString(int x, int y, const std::string& s, const Color color, int size)
{
	int charIndex = 0;
	for (char c : s)
	{
		x += DrawCharacter(x / size, y / size, c, color, charIndex, size) + 1;
		charIndex++;
	}
}
