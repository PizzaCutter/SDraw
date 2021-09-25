#include "SEngine.h"

// INCLUDES FOR GDIPLUS
#include <objidl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")
// ~INCLUDES FOR GDIPLUS

#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono;

#define MAX_LOADSTRING 100

void Start();
void UpdateGame();

static unique_ptr<Gdiplus::Bitmap> bitmap;
static unique_ptr<Gdiplus::Graphics> graphics;
static unique_ptr<Gdiplus::Bitmap> bitmapOther;
static unique_ptr<Gdiplus::Graphics> graphicsOther;

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

void SetPixel(int x, int y, Color c)
{
	Gdiplus::SolidBrush solidBrush(c);
	graphics->FillRectangle(&solidBrush, x, y, 1, 1);
}

void DrawRectangle(int x, int y, int width, int height, Color c)
{
	Gdiplus::SolidBrush solidBrush(c);
	graphics->FillRectangle(&solidBrush, x, y, width, height);
}

int APIENTRY wWinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPWSTR    lpCmdLine,
					  int nCmdShow)
{
	WNDCLASSW wc{ CS_OWNDC, WndProc, 0, 0, hInstance, LoadIcon(nullptr, IDI_APPLICATION), LoadCursor(nullptr, IDC_ARROW), (HBRUSH)(COLOR_WINDOW + 1), nullptr, L"SEngine" };
	if (!RegisterClassW(&wc)) return 1;

	const DWORD style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
	
	RECT r{ 0, 0, Width * PixelScale, Height * PixelScale};
	AdjustWindowRect(&r, style, FALSE);
	
	HWND wnd = CreateWindowW(L"SEngine", L"SEngine", style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top, nullptr, nullptr, hInstance, nullptr);
	if (wnd == nullptr) return 1;

	// Initialize GDI+.
	ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	bitmap = make_unique<Gdiplus::Bitmap>(Width, Height);
	graphics = make_unique<Gdiplus::Graphics>(bitmap.get());
	bitmapOther = make_unique<Gdiplus::Bitmap>(Width, Height);
	graphicsOther = make_unique<Gdiplus::Graphics>(bitmapOther.get());

	Clear(Blue);

	ShowWindow(wnd, nCmdShow);
	UpdateWindow(wnd);

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

		const auto now = high_resolution_clock::now();
		const auto timeBetweenFrames = now - lastDraw;
		const auto framerate = 0.16ms;
		if (timeBetweenFrames > framerate)
		{
			UpdateGame();
			InvalidateRect(wnd, nullptr, false);
			
			lastDraw = now;
		}else
		{
			std::this_thread::sleep_for(1ms);
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
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
