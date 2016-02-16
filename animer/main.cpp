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

float zoomLevel = 1.0f;
int xOffset = 0;
int yOffset = 0;
int currentPaintIndex = 0;

int currMouseX = 0;
int currMouseY = 0;
int mouseState = 0;

void RenderGradient();
void WindowsPaintWindow(HWND hwnd);
void MouseDown(int mouseX, int mouseY);

typedef struct{
	char* fileName;
	BitmapData spriteData;
	int duration;
} AnimKeyFrame;

typedef struct{
	char* name;
	AnimKeyFrame* keyFrames;
	int keyFrameCount;
} AnimClip;

typedef struct{
	AnimClip* animClips;
	int animClipCount;
} AnimAsset;

void AddAnimClip(AnimAsset* asset, AnimClip clip){
	AnimClip* newAnimClips = (AnimClip*)malloc(sizeof(AnimClip)*(asset->animClipCount+1));
	memcpy(newAnimClips, asset->animClips, asset->animClipCount*sizeof(AnimClip));
	newAnimClips[asset->animClipCount] = clip;
	
	free(asset->animClips);
	asset->animClips = newAnimClips;
	asset->animClipCount++;
}

void AddKeyFrame(AnimClip* clip, AnimKeyFrame keyFrame){
	AnimKeyFrame* newKeyFrames = (AnimKeyFrame*)malloc(sizeof(AnimKeyFrame)*(clip->keyFrameCount+1));
	memcpy(newKeyFrames, clip->keyFrames, clip->keyFrameCount*sizeof(AnimKeyFrame));
	newKeyFrames[clip->keyFrameCount] = keyFrame;
	
	free(clip->keyFrames);
	clip->keyFrames = newKeyFrames;
	clip->keyFrameCount++;
}

void ReadAnimAssetFile(AnimAsset* asset, char* fileName, char* dirName, int dirLength);

void SaveAnimAssetFile(AnimAsset* asset, char* fileName);

int CALLBACK WinMain(HINSTANCE inst, HINSTANCE prevInst,
					 LPSTR cmdLine, int cmdShow) {
	
	
	WNDCLASS windowCls = {};
	windowCls.hInstance = inst;
	windowCls.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowCls.lpfnWndProc = MyGuiWindowProc;
	windowCls.lpszClassName = "my-gui-window";

	RegisterClass(&windowCls);

	HWND window = CreateWindow(windowCls.lpszClassName, "Animer", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 50, 50, 1280, 720, 0, 0, inst, 0);

	Timer timer;

	char* commandLine = cmdLine;
	char* arg1Str = commandLine + strspn(commandLine, "\n ");
	int arg1Length = strcspn(arg1Str, "\n ");
	
	InitText("C:/Program Files/Java/jdk1.8.0_65/jre/lib/fonts/LucidaSansRegular.ttf", 18);

	AnimAsset animAsset = {};
	
	ReadAnimAssetFile(&animAsset, "anim.txt", arg1Str, arg1Length);
	
	int animClipIndex = 0;
	
	float animTime = 0.0f;
	
	bool isPlaying = true;
	
	int white = 0xFFFFFFFF;
	BitmapData whiteCol = {&white, 1, 1};
	
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

		if(isPlaying){
			animTime += deltaTime;
		}
		
		if(mouseState > 1){
			MouseDown(currMouseX, currMouseY);
		}
		
		if (keyStates['W'] > 1) {
			char fullAssetFileName[256] = {};
			sprintf(fullAssetFileName, "%.*s/%s", arg1Length, arg1Str, "anim.txt");
			SaveAnimAssetFile(&animAsset, fullAssetFileName);
		}
		
		BitmapData frameBuffer = { (int*)bitmapData, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight };
		memset(bitmapData, 0, frameBuffer.width*frameBuffer.height * 4);

		DrawBox(frameBuffer, 20, frameBuffer.height - 40, frameBuffer.width - 220, 30, white);
		
		int frameLength = 0;
		for(int i = 0; i < animAsset.animClips[animClipIndex].keyFrameCount; i++){
			frameLength += animAsset.animClips[animClipIndex].keyFrames[i].duration;
		}
		
		float animLengthInSeconds = ((float)frameLength)/60.0f;
		
		while(animTime > animLengthInSeconds){
			animTime -= animLengthInSeconds;
		}
		
		int secondLength = (frameLength+59)/60;
		
		int timelinePixelWidth = frameBuffer.width - 240;
		
		for(int i = 0; i <= secondLength; i++){
			int x = 20 + (timelinePixelWidth/secondLength)*i;
			DrawBox(frameBuffer, x, frameBuffer.height - 80, 20, 70, white);
		}
		
		int animTickX = (animTime/animLengthInSeconds)*timelinePixelWidth;
		DrawBox(frameBuffer, animTickX, frameBuffer.height - 90, 20, 80, 0xFF7777FF);
		
		DrawText(frameBuffer, (isPlaying ? "Pause" : "Play"), frameBuffer.width - 180, frameBuffer.height - 120, 150, 40);
		if(Button(frameBuffer, frameBuffer.width - 180, frameBuffer.height - 80, 100, 50, 0x777777, 0xEEEEEE, 0xDDDDDD, "")){
			isPlaying = !isPlaying;
		}
		
		if(mouseState == HOLD && currMouseX > 20 && currMouseX < frameBuffer.width - 220 
			&& currMouseY > frameBuffer.height - 100 && currMouseY < frameBuffer.height){
			float projectedAnimTime = ((float)currMouseX - 10)/timelinePixelWidth * animLengthInSeconds;
			
			animTime = clamp(projectedAnimTime, 0.0f, animLengthInSeconds);
		}
		
		int currentFrames = (int)(animTime*60);
		
		int frames = 0;
		int currKeyFrame = 0;
		for(int i = 0; i < animAsset.animClips[animClipIndex].keyFrameCount; i++){
			frames += animAsset.animClips[animClipIndex].keyFrames[i].duration;
			
			if(frames > currentFrames){
				currKeyFrame = i;
				break;
			}
		}
		
		int startTime = 0;
		for(int i = 0; i < animAsset.animClips[animClipIndex].keyFrameCount; i++){
			float startPixels = ((float)startTime)/animLengthInSeconds * timelinePixelWidth;
			DrawBitmap(frameBuffer, startPixels, frameBuffer.height - 150, 50, 50, animAsset.animClips[animClipIndex].keyFrames[i].spriteData);
			
			startTime += animAsset.animClips[animClipIndex].keyFrames[i].duration;
		}
		
		DrawBitmap(frameBuffer, 80, 80, 450, 450, animAsset.animClips[animClipIndex].keyFrames[currKeyFrame].spriteData);
		
		WindowsPaintWindow(window);
		
		if(mouseState == PRESS){
			mouseState = HOLD;
		}
		if(mouseState == RELEASE){
			mouseState = OFF;
		}

		for(int i = 0; i < 256; i++){
			if(keyStates[i] == PRESS){
				keyStates[i] = HOLD;
			}
			if(keyStates[i] == RELEASE){
				keyStates[i] = OFF;
			}
		}
		
		zoomLevel = clamp(zoomLevel, 0.25f, 2.0f);
	}

	return 0;
}

void ReadAnimAssetFile(AnimAsset* asset, char* fileName, char* dirName, int dirLength){
	char fullFileName[256] = {};
	sprintf(fullFileName, "%.*s/%s", dirLength, dirName, fileName);
	
	FILE* animAssetFile = fopen(fullFileName, "rb");
	
	fseek(animAssetFile, 0, SEEK_END);
	int fileSize = ftell(animAssetFile);
	fseek(animAssetFile, 0, SEEK_SET);
	
	char* fileBuffer = (char*)malloc(fileSize+1);
	fread(fileBuffer, 1, fileSize, animAssetFile);
	fileBuffer[fileSize] = '\0';
	
	fclose(animAssetFile);
	
	char* fileCursor = fileBuffer;
	while(fileCursor != NULL && fileCursor - fileBuffer < fileSize){
		fileCursor += strspn(fileCursor, " \n\r\t");
		
		AnimClip animClip = {};
		
		char* varNameStart = fileCursor;
		fileCursor += strcspn(fileCursor, ": \n\r\t");
		char* varNameEnd = fileCursor;
		
		int varNameLength = varNameEnd - varNameStart;
		char* varName = (char*)malloc(varNameLength + 1);
		memcpy(varName, varNameStart, varNameLength);
		varName[varNameLength] = '\0';
		
		animClip.name = varName;
		
		fileCursor += strspn(fileCursor, ": \n\r\t");
		
		char* nextNewLine = strstr(fileCursor, "\n");
		if(nextNewLine == NULL){
			nextNewLine = &fileBuffer[fileSize];
		}
		
		while(fileCursor != NULL && fileCursor < nextNewLine){
			fileCursor += strspn(fileCursor, ": \t");
			char* fileNameStart = fileCursor;
			fileCursor += strcspn(fileCursor, " ;\t\n\r");
			char* fileNameEnd = fileCursor;
			
			fileCursor += strspn(fileCursor, " ;\t");
			
			char* keyLengthStart = fileCursor;
			
			fileCursor += strcspn(fileCursor, " ,;\t\n\r");
			char* keyLengthEnd = fileCursor;

			fileCursor++;
			
			if(keyLengthStart != keyLengthEnd){
				AnimKeyFrame keyFrame;
				
				int spriteFileNameLength = fileNameEnd - fileNameStart;
				char* spriteFileName = (char*)malloc(spriteFileNameLength+1);
				memcpy(spriteFileName, fileNameStart, spriteFileNameLength);
				spriteFileName[spriteFileNameLength] = '\0';
				
				keyFrame.fileName = spriteFileName;
				keyFrame.duration = atoi(keyLengthStart);
				
				char spriteFullFileName[256] = {};
				sprintf(spriteFullFileName, "%.*s/%s", dirLength, dirName, spriteFileName);
				keyFrame.spriteData = LoadBMPFile(spriteFullFileName);
				
				if(keyFrame.spriteData.data == NULL){
					free(spriteFileName);
				}
				else{
					AddKeyFrame(&animClip, keyFrame);
				}
			}
			else{
				break;
			}
		}
		
		AddAnimClip(asset, animClip);
		
		fileCursor += strspn(fileCursor, " \n\r\t");
	}
	
	free(fileBuffer);
}

void SaveAnimAssetFile(AnimAsset* asset, char* fileName){
	FILE* animAssetFile = fopen(fileName, "wb");
	
	for(int i = 0; i < asset->animClipCount; i++){
		fprintf(animAssetFile, "%s:", asset->animClips[i].name);
		for(int j = 0; j < asset->animClips[i].keyFrameCount; j++){
			fprintf(animAssetFile, "%s%s;%d", (j == 0 ? "" : ","), 
					asset->animClips[i].keyFrames[j].fileName, 
					asset->animClips[i].keyFrames[j].duration);
		}
		
		fprintf(animAssetFile, "\n");
	}
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