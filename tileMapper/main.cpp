#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef DrawText

#include "Renderer.h"
#include "Timer.h"
#include "openfile.h"

#define TILE_INDEX_MULTIPLIER 24

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
	return (KeyState)((wasDown ? 2 : 0) | (isDown ? 1 : 0));
}

#include "BackgroundParsing.h"

BackgroundAsset bgAsset = {};

BitmapData backMap = {};

void* bitmapData = NULL;

struct WindowObj;

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

BitmapData* bgSprites = NULL;
int bgSpriteCount = 0;

const char* arg1Str = "";
int arg1Length = 0;

void MouseDown(int mouseX, int mouseY);

void AddBackgroundSprite(BackgroundAsset* bgAsset, BitmapData** bgSpriteList, int* bgSpriteCount, const char* fileName, const char* dirName, int dirNameLength){
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


void Init(){
	char backgroundAssetFileName[256] = {};
	int backgroundAssetFileNameLength = snprintf(backgroundAssetFileName, 256, "%.*s/background.txt", arg1Length, arg1Str);
	
	bgAsset = ParseBGAssetFile(backgroundAssetFileName);
	
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
}
	
	//InitText("C:/Program Files/Java/jdk1.8.0_65/jre/lib/fonts/LucidaSansRegular.ttf", 18);

void RunFrame(){

	/*
	double deltaTime = timer.GetTimeSince();
	timer.Reset();
	char timeStr[256] = {};
	snprintf(timeStr, 255, "Time this frame: %3.3f ms\n", deltaTime*1000);
	*/
	BitmapData frameBuffer = { (int*)bitmapData, frameWidth, frameHeight };
	
	if(mouseState > 1){
		MouseDown(currMouseX, currMouseY);
	}

	
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

	DrawText(frameBuffer,  "Load New BMP", frameBuffer.width - 150, frameBuffer.height - 150, 120, 40);

	if(Button(frameBuffer, frameBuffer.width - 150, frameBuffer.height - 100, 120, 40, 0x55555555, 0xDDDDDDDD, 0xBBBBBBBB, "")){

		char fileName[256] = {};
		if(OpenFile(fileName, windowObj, "bmp", "BMP files(*.bmp)", arg1Str, arg1Length)){
			AddBackgroundSprite(&bgAsset, &bgSprites, &bgSpriteCount, fileName, arg1Str, arg1Length);
		}
	}
	
	for(int i = 0; i < 256; i++){
		if(keyStates[i] == 1){
			printf("Key '%d' on this side of things was pressed\n", i);
		}
	}
	
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
		
		char backgroundAssetFileName[256] = {};
		snprintf(backgroundAssetFileName, 256, "%.*s/background.txt", arg1Length, arg1Str);
	
		WriteBGAssetFile(bgAsset, backgroundAssetFileName);
	}

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

void MouseDown(int mouseX, int mouseY) {
	int tileSize = (int)(16 * zoomLevel);
	
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

