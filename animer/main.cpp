#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../toolsCode/Renderer.h"
#include "../toolsCode/Timer.h"
#include "../toolsCode/openfile.h"

#define TILE_INDEX_MULTIPLIER 24

void* bitmapData = NULL;

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))

#define SAFE_FREE(x) {if(x){free(x); (x) = nullptr;}}

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

struct WindowObj;

const char* arg1Str;
int arg1Length;

WindowObj* windowObj;

int frameWidth = 0;
int frameHeight = 0;

float zoomLevel = 1.0f;
int xOffset = 0;
int yOffset = 0;
int currentPaintIndex = 0;

int currMouseX = 0;
int currMouseY = 0;
int mouseState = 0;

#include "AnimAsset.h"

AnimAsset animAsset = {};
int animClipIndex = 0;
float animTime = 0.0f;
bool isPlaying = true;
int white = 0xFFFFFFFF;
BitmapData whiteCol = {&white, 1, 1};

int circleData[256] = {};
BitmapData cyanCircle = {circleData, 16, 16};

int plusData[256] = {};
BitmapData plusSign = {plusData, 16, 16};

int crossData[256] = {};
BitmapData crossSign = {crossData, 16, 16};

Timer timer;
	
void Init(){
	ReadAnimAssetFile(&animAsset, "anim.txt", arg1Str, arg1Length);
	
	for(int i = 0; i < 16; i++){
		crossData[17*i] = 0xEE2222;
		crossData[16*i+16-i] = 0xEE2222;
	}
	
	for(int i = 0; i < 16; i++){
		plusData[128+i] = 0xFFFFFF;
		plusData[i*16+8] = 0xFFFFFF;
	}
	
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
}

void RunFrame(){

	float deltaTime = (float)timer.GetTimeSince();
	timer.Reset();

	if(isPlaying){
		animTime += deltaTime;
	}
	
	if (keyStates['W'] > 1) {
		char fullAssetFileName[256] = {};
		sprintf(fullAssetFileName, "%.*s/%s", arg1Length, arg1Str, "anim.txt");
		SaveAnimAssetFile(&animAsset, fullAssetFileName);
	}
	
	BitmapData frameBuffer = { (int*)bitmapData, frameWidth, frameHeight };
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
				if(OpenFile(fileName, windowObj, "bmp", "BMP files(*.bmp)", arg1Str, arg1Length)){
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
		if(OpenFile(fileName, windowObj, "bmp", "BMP files(*.bmp)", arg1Str, arg1Length)){
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