#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Renderer.h"
#include "Timer.h"

#define TILE_INDEX_MULTIPLIER 24

LRESULT CALLBACK MyGuiWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HBITMAP bitmap = 0;
void* bitmapData = NULL;
BITMAPINFO bmpInfo = {};

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#define SAFE_FREE(x) {if(x){free(x); (x) = nullptr;}}
#define SAFE_DELETE(x) {if(x){delete (x); (x) = nullptr;}}
#define SAFE_DELETE_ARR(x) {if(x){delete[] (x); (x) = nullptr;}}

typedef struct {
	char* start;
	int length;
}Token;

inline float clamp(float val, float min, float max) {
	return MIN(max, MAX(min, val));
}

inline int clamp(int val, int min, int max) {
	return MIN(max, MAX(min, val));
}

inline bool RangeCheck(int a, int b, int c) {
	return (a < b) && (b < c);
}
 
enum KeyState {
	DOWN = 0,
	RELEASE = 1,
	PRESS = 2,
	HOLD = 3
};

KeyState keyStates[256] = {};

inline KeyState StateFromBools(bool wasDown, bool isDown) {
	return (KeyState)((wasDown ? 1 : 0) | (isDown ? 2 : 0));
}

struct BackgroundAsset {
	char* backMap;
	char* backMapFile;
	char** sprites;
	int spriteCount;
};

BitmapData backMap = {};

float zoomLevel = 1.0f;
int xOffset = 0;
int yOffset = 0;
int currentPaintIndex = 0;

BitmapData* bgSprites = NULL;
int bgSpriteCount = 0;

void RenderGradient();
void WindowsPaintWindow(HWND hwnd);

BackgroundAsset ParseBGAssetFile(char* fileName) {
	FILE* bgFile = NULL;
	fopen_s(&bgFile, fileName, "rb");

	char* whitespace = "\n\r\t ";
	char* spaceAndColon = "\t :";
	char* spaceAndComma = "\t ,";

	if (bgFile != NULL) {
		fseek(bgFile, 0, SEEK_END);
		size_t fileSize = ftell(bgFile);
		fseek(bgFile, 0, SEEK_SET);

		char* fileBuffer = (char*)malloc(fileSize + 1);
		fread(fileBuffer, 1, fileSize, bgFile);
		fileBuffer[fileSize] = '\0';
		fclose(bgFile);

		BackgroundAsset bgAsset = {};

		char* backMapStart = fileBuffer + strspn(fileBuffer, whitespace);
		int backMapLength = strcspn(backMapStart, spaceAndColon);

		bgAsset.backMap = (char*)malloc(backMapLength+1);
		memcpy(bgAsset.backMap, backMapStart, backMapLength);
		bgAsset.backMap[backMapLength] = '\0';

		char* backMapEnd = backMapStart + backMapLength;
		char* backMapFileStart = backMapEnd + strspn(backMapEnd, spaceAndColon);
		int backMapFileLength = strcspn(backMapFileStart, spaceAndColon);

		bgAsset.backMapFile = (char*)malloc(backMapFileLength + 1);
		memcpy(bgAsset.backMapFile, backMapFileStart, backMapFileLength);
		bgAsset.backMapFile[backMapFileLength] = '\0';
		
		char* bgCursor = backMapFileStart + backMapFileLength + 1;
		char* nextComma = bgCursor;
		char* nextNewline = strstr(bgCursor, "\n");
		while (nextComma != NULL && (nextComma < nextNewline || nextNewline == NULL)) {
			bgCursor += strspn(bgCursor, whitespace);

			char* endOfFileName = bgCursor + strcspn(bgCursor, " \t\n\r,");
			Token spriteFileName;
			spriteFileName.start = bgCursor;
			spriteFileName.length = endOfFileName - bgCursor;

			char** newSpriteNames = (char**)malloc((bgAsset.spriteCount+1)*sizeof(char*));
			memcpy(newSpriteNames, bgAsset.sprites, bgAsset.spriteCount*sizeof(char*));

			newSpriteNames[bgAsset.spriteCount] = (char*)malloc(spriteFileName.length+1);
			memcpy(newSpriteNames[bgAsset.spriteCount], spriteFileName.start, spriteFileName.length);
			newSpriteNames[bgAsset.spriteCount][spriteFileName.length] = '\0';
			SAFE_FREE(bgAsset.sprites);
			bgAsset.sprites = newSpriteNames;

			nextComma = strstr(bgCursor, ",");
			nextNewline = strstr(bgCursor, "\n");
			bgCursor = nextComma + 1;
			bgAsset.spriteCount++;
		}

		free(fileBuffer);

		return bgAsset;
	}
	else {
		printf("Could not open bg file '%s'.", fileName);
		return{};
	}
}

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prevInst,
					 LPSTR cmdLine, int cmdShow) {
	
	
	WNDCLASS windowCls = {};
	windowCls.hInstance = inst;
	windowCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowCls.lpfnWndProc = MyGuiWindowProc;
	windowCls.lpszClassName = "my-gui-window";

	RegisterClass(&windowCls);

	HWND window = CreateWindow(windowCls.lpszClassName, "Tile Mapper", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, 1280, 720, 0, 0, inst, 0);

	Timer timer;

	char* commandLine = cmdLine;
	char* arg1Str = commandLine + strspn(commandLine, "\n ");
	int arg1Length = strcspn(arg1Str, "\n ");

	char backgroundAssetFileName[256] = {};
	int backgroundAssetFileNameLength = snprintf(backgroundAssetFileName, 256, "%.*s/background.txt", arg1Length, arg1Str);
	
	BackgroundAsset bgAsset = ParseBGAssetFile(backgroundAssetFileName);
	
	char bgMapFileName[256] = {};
	snprintf(bgMapFileName, 256, "%.*s/%s", arg1Length, arg1Str, bgAsset.backMapFile);

	backMap = LoadBMPFile(bgMapFileName);

	bgSpriteCount = bgAsset.spriteCount;
	bgSprites = (BitmapData*)malloc(sizeof(BitmapData)*bgSpriteCount);
	for (int i = 0, spriteIndex = 0; i < bgAsset.spriteCount; i++, spriteIndex++) {
		char bgSpriteName[256] = {};
		snprintf(bgSpriteName, 256, "%.*s/%s", arg1Length, arg1Str, bgAsset.sprites[i]);
		bgSprites[spriteIndex] = LoadBMPFile(bgSpriteName);

		if (bgSprites[i].data == NULL) {
			spriteIndex--;
			bgSpriteCount--;
		}
	}

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

		BitmapData frameBuffer = { bitmapData, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight };
		memset(bitmapData, 0, frameBuffer.width*frameBuffer.height * 4);

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
					int spriteIdx = ((int*)backMap.data)[pixelIdx] / TILE_INDEX_MULTIPLIER;
					if (spriteIdx > 0) {
						BitmapData sprite = bgSprites[spriteIdx - 1];
						DrawBitmap(frameBuffer, i * tileSize + pixelOffsetX, j * tileSize + pixelOffsetY, tileSize, tileSize, sprite);
					}
				}
			}
		}

		for (int i = 0; i < bgSpriteCount; i++) {
			DrawBitmap(frameBuffer, (i+1) * 32, frameBuffer.height - 32, 32, 32, bgSprites[i]);
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
			zoomLevel /= 1.01f;
		}
		if (keyStates['Z'] > 1) {
			zoomLevel *= 1.01f;
		}

		if (keyStates['F'] == PRESS) {
			for (int i = 0; i < backMap.width*backMap.height; i++) {
				((int*)backMap.data)[i] = currentPaintIndex * TILE_INDEX_MULTIPLIER;
			}
		}

		if (keyStates['O'] == PRESS) {
			BitmapData backMapCopy = {};
			backMapCopy.width = backMap.width;
			backMapCopy.height = backMap.height;
			backMapCopy.data = malloc(backMapCopy.width*backMapCopy.height * 4);

			for (int i = 0; i < backMapCopy.width * backMapCopy.height; i++) {
				int x = (i % backMapCopy.width);
				int y = (i / backMapCopy.width);
				int cpyIdx = x + (backMapCopy.height - 1 - y)*backMapCopy.width;
				((int*)backMapCopy.data)[cpyIdx] = ((int*)backMap.data)[i];// TILE_INDEX_MULTIPLIER;
			}

			char bgMapFile[256] = {};
			snprintf(bgMapFile, 256, "%.*s/%s", arg1Length, arg1Str, bgAsset.backMapFile);
			WriteBMPFile(bgMapFile, &backMapCopy);

			free(backMapCopy.data);
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

void MouseDown(int mouseX, int mouseY) {
	int tileSize = (int)(16 * zoomLevel);
	int frameWidth = bmpInfo.bmiHeader.biWidth;
	int frameHeight = bmpInfo.bmiHeader.biHeight;

	int backMapX = (mouseX + xOffset) / tileSize;
	int backMapY = (mouseY + yOffset) / tileSize;

	if (RangeCheck(-1, backMapX, backMap.width) && RangeCheck(-1, backMapY, backMap.height) && RangeCheck(0, mouseY, frameHeight - 32)){
		int backMapIdx = backMap.width * backMapY + backMapX;
		((int*)backMap.data)[backMapIdx] = currentPaintIndex * TILE_INDEX_MULTIPLIER;
	}
	else if (RangeCheck(frameHeight-33, mouseY, frameHeight)) {
		currentPaintIndex = clamp(mouseX / 32, 0, bgSpriteCount+1);
	}
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

		case WM_MOUSEMOVE:
		{
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);

			if (wParam & MK_LBUTTON) {
				MouseDown(mouseX, mouseY);
			}
		}break;

		case WM_LBUTTONDOWN:
		{
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			MouseDown(mouseX, mouseY);
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