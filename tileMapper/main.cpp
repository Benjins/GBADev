#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef DrawText

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
	OFF = 0,
	RELEASE = 1,
	PRESS = 2,
	HOLD = 3
};

KeyState keyStates[256] = {};

inline KeyState StateFromBools(bool wasDown, bool isDown) {
	return (KeyState)((wasDown ? 1 : 0) | (isDown ? 2 : 0));
}

#include "BackgroundParsing.h"

BitmapData backMap = {};

float zoomLevel = 1.0f;
int xOffset = 0;
int yOffset = 0;
int currentPaintIndex = 0;

int currMouseX = 0;
int currMouseY = 0;
int mouseState = 0;

BitmapData* bgSprites = NULL;
int bgSpriteCount = 0;

void RenderGradient();
void WindowsPaintWindow(HWND hwnd);
void MouseDown(int mouseX, int mouseY);

void AddBackgroundSprite(BackgroundAsset* bgAsset, BitmapData** bgSpriteList, int* bgSpriteCount, char* fileName, char* dirName, int dirNameLength){
	BGSprite* newBGSprites = (BGSprite*)malloc((bgAsset->spriteCount+1)*sizeof(BGSprite));
	memcpy(newBGSprites, bgAsset->sprites, sizeof(BGSprite)*bgAsset->spriteCount);
	
	int fileNameLength = strlen(fileName);
	char* spriteFileName = (char*)malloc(fileNameLength+1);
	memcpy(spriteFileName, fileName, fileNameLength);
	spriteFileName[fileNameLength] = '\0';
	
	newBGSprites[bgAsset->spriteCount].fileName = spriteFileName;
	newBGSprites[bgAsset->spriteCount].flags = NONE;
	
	free(bgAsset->sprites);
	
	bgAsset->sprites = newBGSprites;
	bgAsset->spriteCount++;
	
	BitmapData* newBGSpriteList = (BitmapData*)malloc((1+*bgSpriteCount)*sizeof(BitmapData));
	memcpy(newBGSpriteList, *bgSpriteList, sizeof(BitmapData)*(*bgSpriteCount));
	
	char fullFileName[256] = {};
	snprintf(fullFileName, 256, "%.*s\\%s", dirNameLength, dirName, spriteFileName);
	newBGSpriteList[*bgSpriteCount] = LoadBMPFile(fullFileName);
	
	free(*bgSpriteList);
	*bgSpriteList = newBGSpriteList;
	(*bgSpriteCount)++;
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
		snprintf(bgSpriteName, 256, "%.*s/%s", arg1Length, arg1Str, bgAsset.sprites[i].fileName);
		bgSprites[spriteIndex] = LoadBMPFile(bgSpriteName);

		if (bgSprites[i].data == NULL) {
			spriteIndex--;
			bgSpriteCount--;
		}
	}
	
	InitText("C:/Program Files/Java/jdk1.8.0_65/jre/lib/fonts/LucidaSansRegular.ttf", 18);

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

		if(mouseState > 1){
			MouseDown(currMouseX, currMouseY);
		}
		
		BitmapData frameBuffer = { (int*)bitmapData, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight };
		memset(bitmapData, 0, frameBuffer.width*frameBuffer.height * 4);

		float rowCount = (frameBuffer.height - 48) / 16 / zoomLevel;
		float colCount = (frameBuffer.width - 180)  / 16 / zoomLevel;

		int tileSize = (int)(16 * zoomLevel);

		for(int j = -1; j < rowCount + 1; j++){
			for(int i = -1; i < colCount + 1; i++){
				int tileX = i + xOffset / tileSize;
				int tileY = j + yOffset / tileSize;

				int pixelOffsetX = -xOffset % tileSize;
				int pixelOffsetY = -yOffset % tileSize;

				if (tileX >= 0 && tileX < backMap.width && tileY >= 0 && tileY < backMap.height) {
					int pixelIdx = (backMap.height - 1 - tileY)*backMap.width + tileX;
					int spriteIdx = ((int*)backMap.data)[pixelIdx] / TILE_INDEX_MULTIPLIER;
					if (spriteIdx > 0) {
						BitmapData sprite = bgSprites[spriteIdx - 1];
						DrawBitmap(frameBuffer, i * tileSize + pixelOffsetX, j * tileSize + pixelOffsetY, tileSize, tileSize, sprite);
					}
				}
			}
		}
		
		int backMapX = (currMouseX + xOffset) / tileSize;
		int backMapY = (currMouseY + yOffset) / tileSize;
		
		int* frameMem = (int*)frameBuffer.data;
		
		if(backMapX >= 0 && backMapX < backMap.width && backMapY >= 0 && backMapY < backMap.height){
			int xMin = backMapX * tileSize  - xOffset;
			int xMax = (backMapX+1) * tileSize  - xOffset;
			int yMin = backMapY * tileSize  - yOffset;
			int yMax = (backMapY+1) * tileSize  - yOffset;
			
			xMin = clamp(xMin, 0, frameBuffer.width - 1);
			xMax = clamp(xMax, 0, frameBuffer.width - 1);
			yMin = clamp(yMin, 0, frameBuffer.height - 1);
			yMax = clamp(yMax, 0, frameBuffer.height - 1);
			
			for(int j = yMin; j < yMax; j++){
				for(int i = xMin; i < xMax; i++){
					int frameIdx = (frameBuffer.height - j - 1)*frameBuffer.width+i;
					frameMem[frameIdx] ^= 0xFFFFFF;
				}
			}
		}
		
		int redCol = 0xFFFFFF;
		BitmapData redBMP = {&redCol, 1, 1};
		DrawBitmap(frameBuffer, currentPaintIndex*34,frameBuffer.height - 34, 34, 34, redBMP);
		
		
		for (int i = 0; i < bgSpriteCount; i++) {
			DrawBitmap(frameBuffer, (i+1) * 34 + 1, frameBuffer.height - 33, 32, 32, bgSprites[i]);
		}
		
		//DrawBitmap(frameBuffer, 100, 100, 400, 220, sprite);
		//DrawText(frameBuffer, "The only way.", 500, 200, 200, 60);		
		//DrawText(frameBuffer, "Hello to all, and to all BLVCK.", 300, 500, 120, 80);
		
		if(currentPaintIndex > 0){
			
			int flags = bgAsset.sprites[currentPaintIndex-1].flags;
			
			int index = 0;
			for(int i = 1; i < MAX_SPRITE_FLAG; i *= 2){
				
				char flagText[256] = {};
				sprintf(flagText, "Flag: %d, val: %s", i,  (flags & i) ? "True" : "False");
				DrawText(frameBuffer, flagText, frameBuffer.width - 150, 200 + index*30, 150, 30);
				
				if(Button(frameBuffer, frameBuffer.width - 150, 200 + index*30, 60, 10, 0x55555555, 0xDDDDDDDD, 0xBBBBBBBB, "")){
					flags ^= i;
					bgAsset.sprites[currentPaintIndex-1].flags = (BGSpriteFlags)flags;
				}
				
				index++;
			}
		}
		
#if 0
		//Useful for debugging issues with flags and tile indices.
		char debugPaintIndex[256] = {};
		sprintf(debugPaintIndex, "Paint Index: %d", currentPaintIndex);
		DrawText(frameBuffer, debugPaintIndex, frameBuffer.width - 150, 400, 150, 30);
		
		char debugFlagString[256] = {};
		char* flagStrPtr = debugFlagString;
		flagStrPtr += sprintf(flagStrPtr, "Flags: ");
		for(int i = 0; i < bgAsset.spriteCount; i++){
			flagStrPtr += sprintf(flagStrPtr, "%d, ", bgAsset.sprites[i].flags);
		}
		
		DrawText(frameBuffer, debugFlagString, frameBuffer.width - 180, 450, 150, 80);
#endif

		DrawText(frameBuffer,  "Load New BMP", frameBuffer.width - 150, frameBuffer.height - 150, 120, 40);
		
		if(Button(frameBuffer, frameBuffer.width - 150, frameBuffer.height - 100, 120, 40, 0x55555555, 0xDDDDDDDD, 0xBBBBBBBB, "")){
			OPENFILENAME spriteFile = {};
			spriteFile.hInstance = inst;
			
			char initialDir[256] = {};
			GetCurrentDirectory(sizeof(initialDir), initialDir);
			char usedDir[256] = {};
			snprintf(usedDir, 256, "%s\\%.*s", initialDir, arg1Length, arg1Str);
			
			spriteFile.lpstrInitialDir = usedDir;

			char szFilters[] = "Bitmap Files (*.bmp)\0*.bmp\0\0";
			char szFilePathName[512] = "";

			spriteFile.lStructSize = sizeof(OPENFILENAME);
			spriteFile.hwndOwner = window;
			spriteFile.lpstrFilter = szFilters;
			spriteFile.lpstrFile = szFilePathName;  // This will hold the file name
			spriteFile.lpstrDefExt = "bmp";
			spriteFile.nMaxFile = 512;
			spriteFile.lpstrTitle = "Open BMP File";
			spriteFile.Flags = OFN_OVERWRITEPROMPT;
			
			if(GetOpenFileName(&spriteFile)){	
				//The open file dialog sets the current directory to the project directory,
				//so we reset it to the outer directory
				SetCurrentDirectory(initialDir);
				
				char shortFileName[256] = {};
				int fullPathLength = strlen(szFilePathName);
				char* fileNameCursor = szFilePathName + (fullPathLength - 1);
				for(int i = 0; i < fullPathLength - 1; i++){
					fileNameCursor--;
					if(*fileNameCursor == '\\' || *fileNameCursor == '/'){
						fileNameCursor++;
						break;
					}
				}

				AddBackgroundSprite(&bgAsset, &bgSprites, &bgSpriteCount, fileNameCursor, arg1Str, arg1Length);
			}
		}

		
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
			backMapCopy.data = (int*)malloc(backMapCopy.width*backMapCopy.height * 4);

			for (int i = 0; i < backMapCopy.width * backMapCopy.height; i++) {
				((int*)backMapCopy.data)[i] = ((int*)backMap.data)[i];// TILE_INDEX_MULTIPLIER;
			}

			char bgMapFile[256] = {};
			snprintf(bgMapFile, 256, "%.*s/%s", arg1Length, arg1Str, bgAsset.backMapFile);
			WriteBMPFile(bgMapFile, &backMapCopy);

			free(backMapCopy.data);
			
			WriteBGAssetFile(bgAsset, backgroundAssetFileName);
		}
		
		if(mouseState = PRESS){
			mouseState = HOLD;
		}
		if(mouseState = RELEASE){
			mouseState = OFF;
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
	
	currMouseX = mouseX;
	currMouseY = mouseY;

	int backMapX = (mouseX + xOffset) / tileSize;
	int backMapY = (mouseY + yOffset) / tileSize;

	if (RangeCheck(-1, backMapX, backMap.width) && RangeCheck(-1, backMapY, backMap.height) && RangeCheck(0, mouseY, frameHeight - 32)){
		int backMapIdx = backMap.width * (backMap.height - 1 - backMapY) + backMapX;
		((int*)backMap.data)[backMapIdx] = currentPaintIndex * TILE_INDEX_MULTIPLIER;
	}
	else if (RangeCheck(frameHeight-33, mouseY, frameHeight)) {
		currentPaintIndex = clamp(mouseX / 34, 0, bgSpriteCount);
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
			
			currMouseX = mouseX;
			currMouseY = mouseY;

			if (wParam & MK_LBUTTON) {
				mouseState = HOLD;
			}
		}break;

		case WM_LBUTTONDOWN:
		{
			int mouseX = GET_X_LPARAM(lParam);
			int mouseY = GET_Y_LPARAM(lParam);
			
			if(mouseState != HOLD){
				mouseState = PRESS;
			}
		}break;
		
		case WM_LBUTTONUP:
		{
			//int mouseX = GET_X_LPARAM(lParam);
			//int mouseY = GET_Y_LPARAM(lParam);
			
			mouseState = RELEASE;
			//MouseDown(mouseX, mouseY);
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