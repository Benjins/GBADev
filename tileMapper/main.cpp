#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Renderer.h"
#include "Timer.h"

LRESULT CALLBACK MyGuiWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HBITMAP bitmap = 0;
void* bitmapData = NULL;
BITMAPINFO bmpInfo = {};

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

inline float clamp(float val, float min, float max) {
	return MIN(max, MAX(min, val));
}

inline int clamp(int val, int min, int max) {
	return MIN(max, MAX(min, val));
}
 
enum KeyState {
	DOWN = 0,
	RELASE = 1,
	PRESS = 2,
	HOLD = 3
};

KeyState keyStates[256] = {};

inline KeyState StateFromBools(bool wasDown, bool isDown) {
	return (KeyState)((wasDown ? 1 : 0) | (isDown ? 2 : 0));
}

BitmapData spriteTiles[128] = {};
int spriteTileCount = 0;
BitmapData backMap = {};

float zoomLevel = 1.0f;
int xOffset = 0;
int yOffset = 0;

void RenderGradient();
void WindowsPaintWindow(HWND hwnd);

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prevInst,
					 LPSTR cmdLine, int cmdShow) {
	

	WNDCLASS windowCls = {};
	windowCls.hInstance = inst;
	windowCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowCls.lpfnWndProc = MyGuiWindowProc;
	windowCls.lpszClassName = "my-gui-window";

	RegisterClass(&windowCls);

	HWND window = CreateWindow(windowCls.lpszClassName, "my-gui", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, 1280, 720, 0, 0, inst, 0);

	Timer timer;

	BitmapData sprite = LoadBMPFile("topDown/shortMap.bmp");
	
	BitmapData bg1 = LoadBMPFile("topDown/bg1.bmp");
	BitmapData bg2 = LoadBMPFile("topDown/bg2.bmp");
	BitmapData bg3 = LoadBMPFile("topDown/bg3.bmp");
	
	BitmapData bgSprites[] = {bg1, bg2, bg3};
	
	backMap = LoadBMPFile("topDown/shortMap.bmp");
	
	int* indices = (int*)malloc(4*backMap.width*backMap.height);
	
	int paletteCols[64] = {};
	int palCount = 1;
	for(int j = 0; j < backMap.height; j++){
		for(int i = 0; i < backMap.width; i++){
			int pixelIdx = j*backMap.width+i;
			
			int palIdx = -1;
			for(int k = 0; k < palCount; k++){
				if(((int*)backMap.data)[pixelIdx] == paletteCols[k]){
					palIdx = k;
					break;
				}
			}
			
			if(palIdx < 0){
				paletteCols[palCount] = ((int*)backMap.data)[pixelIdx];
				palIdx = palCount;
				palCount++;
			}
			
			int indicesIdx = (backMap.height - 1 - j)*backMap.width+i; 
			indices[indicesIdx] = palIdx;
		}
	}
	
	free(backMap.data);
	backMap.data = indices;
	
	bool isRunning = true;
	while (isRunning) {
		tagMSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_QS_INPUT))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//TODO: Why the f--k is this 161, not WM_QUIT?
			if (msg.message == 161 && msg.wParam == 20) {
				isRunning = false;
			}
		}

		double deltaTime = timer.GetTimeSince();
		timer.Reset();
		char timeStr[256] = {};
		snprintf(timeStr, 255, "Time this frame: %3.3f ms\n", deltaTime*1000);
		OutputDebugStringA(timeStr);
		
		//RenderGradient();
		
		BitmapData frameBuffer = { bitmapData, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight };
		memset(bitmapData, 0, frameBuffer.width*frameBuffer.height * 4);

		//Render(frameBuffer);
		float rowCount = (frameBuffer.height - 48) / 16 / zoomLevel;
		float colCount = frameBuffer.width  / 16 / zoomLevel;

		int tileSize = (int)(16 * zoomLevel);

		for(int j = -1; j < rowCount + 1; j++){
			for(int i = -1; i < colCount + 1; i++){
				int tileX = i + xOffset / tileSize;
				int tileY = j + yOffset / tileSize;

				int pixelOffsetX = -xOffset % tileSize;
				int pixelOffsetY = -yOffset % tileSize;

				//if (pixelOffsetX < 0) { pixelOffsetX += 16; }
				//if (pixelOffsetY < 0) { pixelOffsetY += 16; }

				if (tileX >= 0 && tileX < backMap.width && tileY >= 0 && tileY < backMap.height) {
					int pixelIdx = tileY*backMap.width + tileX;
					int spriteIdx = ((int*)backMap.data)[pixelIdx];
					if (spriteIdx > 0) {
						BitmapData sprite = bgSprites[spriteIdx - 1];
						DrawBitmap(frameBuffer, i * tileSize + pixelOffsetX, j * tileSize + pixelOffsetY, tileSize, tileSize, sprite);
					}
				}
			}
		}

		for (int i = 0; i < ARRAY_COUNT(bgSprites); i++) {
			DrawBitmap(frameBuffer, i * 32, frameBuffer.height - 32, 32, 32, bgSprites[i]);
		}
		
		//DrawBitmap(frameBuffer, 100, 100, 400, 220, sprite);
		

		WindowsPaintWindow(window);
		//xOffset++;

		if (keyStates['W'] > 1) {
			yOffset--;
		}
		if (keyStates['S'] > 1) {
			yOffset++;
		}
		if (keyStates['A'] > 1) {
			xOffset--;
		}
		if (keyStates['D'] > 1) {
			xOffset++;
		}
		if (keyStates['Q'] > 1) {
			zoomLevel /= 1.1f;
		}
		if (keyStates['Z'] > 1) {
			zoomLevel *= 1.1f;
		}

		zoomLevel = clamp(zoomLevel, 0.25f, 2.0f);
	}

	return 0;
}

void ResizeWindow(int w, int h) {
	if (bitmap) {
		DeleteObject(bitmap);
	}

	bmpInfo.bmiHeader.biSize = sizeof(bmpInfo.bmiHeader);
	bmpInfo.bmiHeader.biWidth  = w;
	bmpInfo.bmiHeader.biHeight = h;
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biBitCount = 32;
	bmpInfo.bmiHeader.biCompression = BI_RGB;

	HDC deviceContext = CreateCompatibleDC(0);
	bitmap = CreateDIBSection(deviceContext, &bmpInfo, DIB_RGB_COLORS, &bitmapData, 0, 0);
	ReleaseDC(0, deviceContext);
}

void RenderGradient() {
	int width = bmpInfo.bmiHeader.biWidth;
	int height= bmpInfo.bmiHeader.biHeight;

	int* bmpData = (int*)bitmapData;
	for (int j = 0; j < height; j++) {
		for (int i = 0; i < width; i++) {
			int trueX = i + xOffset;
			int trueY = j + yOffset;
			bmpData[j*width + i] = RGB(trueX % 255, trueY % 255, xOffset);
		}
	}

	for (int j = height / 2 - 20; j < height / 2 + 20; j++) {
		for (int i = width / 2 - 20; i < width / 2 + 20; i++) {
			bmpData[j*width + i] = RGB(j % 200, i % 200, i * j % 250);
		}
	}
}

void WindowsPaintWindow(HWND hwnd) {
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	HDC deviceContext = GetDC(hwnd);

	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	int bmpWidth = bmpInfo.bmiHeader.biWidth, bmpHeight = bmpInfo.bmiHeader.biHeight;

	StretchDIBits(deviceContext, 
		0, 0, clientWidth, clientHeight, 
		0, 0, bmpWidth, bmpHeight, 
		bitmapData, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(hwnd, deviceContext);
}

LRESULT CALLBACK MyGuiWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	LRESULT result = 0;
	
	switch (message) {
		case WM_SIZE: {
			RECT clientRect;
			GetClientRect(hwnd, &clientRect);
			ResizeWindow(clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
		}break;
	
		case WM_DESTROY: {
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}break;

		case WM_CLOSE: {
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP: {
			int code = wParam;
			bool wasDown = (lParam & (1 << 30)) != 0;
			bool  isDown = (lParam & (1 << 31)) == 0;

			keyStates[code] = StateFromBools(wasDown, isDown);
		}break;
		
		
		case WM_PAINT: {
			WindowsPaintWindow(hwnd);
		}break;
		

		case WM_ACTIVATEAPP: {
		
		}break;

		case WM_MOUSEWHEEL: {
			float zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			printf("Whell delta: %f\n", zDelta);
			
		}break;

		case WM_GESTURE: {
			printf("WM Gesture.\n");
		}break;

		case WM_TOUCH: {
			printf("WM Touch.\n");
		}break;
		
		default: {
			result = DefWindowProc(hwnd, message, wParam, lParam);
		} break;
	
	}

	return result;
}