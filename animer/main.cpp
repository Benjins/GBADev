#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

int scrollY = 0;

void RenderGradient();
void WindowsPaintWindow(HWND hwnd);
void MouseDown(int mouseX, int mouseY);

#include "AnimAsset.h"

bool OpenFile(char* fileName, HWND owner, char* fileFilter, char* filterTitle, char* dirName, int dirLength){
	OPENFILENAME spriteFile = {};
	
	char initialDir[256] = {};
	GetCurrentDirectory(sizeof(initialDir), initialDir);
	char usedDir[256] = {};
	snprintf(usedDir, 256, "%s\\%.*s", initialDir, dirLength, dirName);
	
	spriteFile.lpstrInitialDir = usedDir;

	char szFilters[256] = {};
	sprintf(szFilters, "%s (*.%s)\0*.%s\0\0", filterTitle, fileFilter, fileFilter);
	char szFilePathName[512] = "";

	spriteFile.lStructSize = sizeof(OPENFILENAME);
	spriteFile.hwndOwner = owner;
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
		
		int fileNameLength = strlen(fileNameCursor);
		memcpy(fileName, fileNameCursor, fileNameLength);
		fileName[fileNameLength] = '\0';
		
		return true;
	}
	
	return false;
}

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
	
	int circleData[256] = {};
	BitmapData cyanCircle = {circleData, 16, 16};
	
	int plusData[256] = {};
	for(int i = 0; i < 16; i++){
		plusData[128+i] = 0xFFFFFF;
		plusData[i*16+8] = 0xFFFFFF;
	}
	
	BitmapData plusSign = {plusData, 16, 16};
	
	int crossData[256] = {};
	for(int i = 0; i < 16; i++){
		crossData[17*i] = 0xEE2222;
		crossData[16*i+16-i] = 0xEE2222;
	}
	
	BitmapData crossSign = {crossData, 16, 16};
	
	for(int j = 0; j < 16; j++){
		float rSqr = 64;
		float heightSqr = (8 - j)*(8 - j);
		float width = sqrt(rSqr - heightSqr);
		for(int i = 0; i < 16; i++){
			int idx = j*16+i;
			
			if(abs(8 - i) < (int)width){
				circleData[idx] = 0xFFFFFF;
			}
			else if(abs(8 - i) < width){
				float widthFrac = width - (int)width;
				unsigned char amt = (unsigned char)(width * 255);
				circleData[idx] = amt | (amt << 8) | (amt << 16);
			}
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
		
		if(keyStates['T'] > 1){
			scrollY++;
		}
		if(keyStates['y'] > 1){
			scrollY--;
		}
		
		BitmapData frameBuffer = { (int*)bitmapData, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight };
		memset(bitmapData, 0, frameBuffer.width*frameBuffer.height * 4);
		
		for(int i = 0; i < animAsset.animClipCount; i++){
			DrawText(frameBuffer, animAsset.animClips[i].name, frameBuffer.width - 180, 100*i+40, 150, 40);
			if(Button(frameBuffer, frameBuffer.width - 180, 100*i+50, 50, 30, 0x777777, 0xEEEEEE, 0xDDDDDD, "")){
				animClipIndex = i;
				animTime = 0;
			}
		}
		
		//Useful for seeing performance
		//DrawText(frameBuffer, timeStr, 400, 400, 500, 100);
		
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
		
		DrawText(frameBuffer, (isPlaying ? "Pause" : "Play"), frameBuffer.width - 180, frameBuffer.height - 90, 150, 40);
		if(Button(frameBuffer, frameBuffer.width - 180, frameBuffer.height - 80, 100, 50, 0x777777, 0xEEEEEE, 0xDDDDDD, "")){
			isPlaying = !isPlaying;
		}
		
		if(mouseState == HOLD && currMouseX > 20 && currMouseX < frameBuffer.width - 220 
			&& currMouseY > frameBuffer.height - 100 && currMouseY < frameBuffer.height){
			float projectedAnimTime = ((float)currMouseX - 10)/timelinePixelWidth * animLengthInSeconds;
			
			animTime = clamp(projectedAnimTime, 0.0f, animLengthInSeconds);
		}
		else if(mouseState == HOLD && currMouseY < frameBuffer.height - 150 
				&& currMouseY > frameBuffer.height - 200){
			int frameLength = 0;
			for(int i = 0; i < animAsset.animClips[animClipIndex].keyFrameCount; i++){	
				float pixelX = (float(frameLength))/60 / animLengthInSeconds *  timelinePixelWidth;
				if(currMouseX > pixelX && currMouseX < pixelX + 50){
					float pixelOffset = pixelX + 25 - currMouseX;
					float frameOffset = pixelOffset / timelinePixelWidth * animLengthInSeconds * 60;
					
					int offsetInt = (frameOffset < 0 ? -1 : 1) * (int)((frameOffset < 0 ? -frameOffset : frameOffset));

					if(i != 0){
						animAsset.animClips[animClipIndex].keyFrames[i-1].duration -= offsetInt;
						if (i != animAsset.animClips[animClipIndex].keyFrameCount - 1) {
							animAsset.animClips[animClipIndex].keyFrames[i].duration += offsetInt;
						}
					}
					
					break;
				}

				frameLength += animAsset.animClips[animClipIndex].keyFrames[i].duration;
			}
		}
		else if(mouseState == RELEASE && currMouseY < frameBuffer.height - 100 
									  && currMouseY > frameBuffer.height - 150){
			
			int startTime = 0;
			for(int i = 0; i < animAsset.animClips[animClipIndex].keyFrameCount; i++){
				float pixelX = ((float)startTime)/60/animLengthInSeconds * timelinePixelWidth;
				
				if(currMouseX > pixelX && currMouseX < pixelX + 50){
					
					char fileName[256] = {};
					if(OpenFile(fileName, window, "bmp", "Bitmap Files", arg1Str, arg1Length)){
						free(animAsset.animClips[animClipIndex].keyFrames[i].spriteData.data);
						free(animAsset.animClips[animClipIndex].keyFrames[i].fileName);
						
						int newFileNameLength = strlen(fileName);
						char* newFileName = (char*)malloc(newFileNameLength+1);
						memcpy(newFileName, fileName, newFileNameLength);
						newFileName[newFileNameLength] = '\0';
						
						animAsset.animClips[animClipIndex].keyFrames[i].fileName = newFileName;
						
						char fullNewFileName[256] = {};
						sprintf(fullNewFileName, "%.*s/%s", arg1Length, arg1Str, newFileName);
						animAsset.animClips[animClipIndex].keyFrames[i].spriteData = LoadBMPFile(fullNewFileName);
					}
				}
				
				
				startTime += animAsset.animClips[animClipIndex].keyFrames[i].duration;
			}
		}
		
		int currentFrames = (int)(animTime*60);
		
		if(Button(frameBuffer, frameBuffer.width - 150, frameBuffer.height - 250, 100, 50, 0x777777, 0xEEEEEE, 0xDDDDDD, "")){
			char fileName[256] = {};
			if(OpenFile(fileName, window, "bmp", "Bitmap Files", arg1Str, arg1Length)){
				int newFileNameLength = strlen(fileName);
				char* newFileName = (char*)malloc(newFileNameLength+1);
				memcpy(newFileName, fileName, newFileNameLength);
				newFileName[newFileNameLength] = '\0';
				
				char fullNewFileName[256] = {};
				sprintf(fullNewFileName, "%.*s/%s", arg1Length, arg1Str, newFileName);
				
				AnimKeyFrame newKeyFrame = {newFileName, LoadBMPFile(fullNewFileName), 20};
				
				AddKeyFrame(&animAsset.animClips[animClipIndex], newKeyFrame);
			}
		}
		DrawBitmap(frameBuffer, frameBuffer.width - 149, frameBuffer.height - 299, 48, 48, plusSign);
		
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
			float startPixels = ((float)startTime)/60/animLengthInSeconds * timelinePixelWidth;
			DrawBitmap(frameBuffer, (int)startPixels, frameBuffer.height - 200, 50, 50, cyanCircle);
			DrawBitmap(frameBuffer, (int)startPixels, frameBuffer.height - 150, 50, 50, animAsset.animClips[animClipIndex].keyFrames[i].spriteData);
			
			int duration = animAsset.animClips[animClipIndex].keyFrames[i].duration;
			
			if(Button(frameBuffer, (int)startPixels, frameBuffer.height - 250, 30, 30, 0x777777, 0xEEEEEE, 0xDDDDDD, "")){
				RemoveAnimKeyFrame(&animAsset.animClips[animClipIndex], i);
			}
			DrawBitmap(frameBuffer, startPixels, frameBuffer.height - 250, 30, 30, crossSign);
			
			startTime += duration;
		}
		
		NoramlizeAnimClip(&animAsset.animClips[animClipIndex]);
		
		DrawBitmap(frameBuffer, 50, 50, 250, 250, animAsset.animClips[animClipIndex].keyFrames[currKeyFrame].spriteData);
		
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