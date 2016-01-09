#include "Renderer.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)

#pragma pack(1)
struct BitMapHeader {
	short fileTag;
	int fileSize;
	short reservedA;
	short reservedB;
	int imageDataOffset;

	int headerSize;
	int imageWidth;
	int imageHeight;
	short numColorPlanes;
	short bitDepth;
	int compressionMethod;
	int imageDataSize;
	int horizontalResolution;
	int verticalResolution;
	int numPaletteColors;
	int numImportantColors;
};
#pragma pack()

#else
typedef struct __attribute((packed))__{
	short fileTag;
	int fileSize;
	short reservedA;
	short reservedB;
	int imageDataOffset;

	int headerSize;
	int imageWidth;
	int imageHeight;
	short numColorPlanes;
	short bitDepth;
	int compressionMethod;
	int imageDataSize;
	int horizontalResolution;
	int verticalResolution;
	int numPaletteColors;
	int numImportantColors;
} BitMapHeader;
#endif

void DrawBox(BitmapData bitmap, int x, int y, int w, int h, int col) {
	int x1 = x;
	int y1 = bitmap.height - y - h;
	int x2 = x + w;
	int y2 = bitmap.height - y;

	if (x2 >= bitmap.width) { x2 = bitmap.width - 1; }
	if (y2 < 0) { y2 = 0; }

	int* pixels = (int*)bitmap.data;

	for (int j = y1; j < y2; j++) {
		for (int i = x1; i < x2; i++) {
			int idx = j*bitmap.width + i;
			pixels[idx] = col;
		}
	}
}

void DrawBitmap(BitmapData bitmap, int x, int y, int w, int h, BitmapData sprite) {
	int x1 = x;
	int y1 = bitmap.height - y - h;
	int x2 = x + w;
	int y2 = bitmap.height - y;

	if (x1 < 0) { x1 = 0; }
	if (y1 < 0) { y1 = 0; }
	if (x2 >= bitmap.width) { x2 = bitmap.width - 1; }
	if (y2 < 0) { y2 = 0; }
	if (y2 >= bitmap.height) { y2 = bitmap.height - 1; }

	int* pixels = (int*)bitmap.data;
	int* spritePixels = (int*)sprite.data;

	float heightRatio = 0.0f;
	for (int j = y1; j < y2; j++, heightRatio += 1.0f/(w)) {
		float widthRatio = 0.0f;
		for (int i = x1; i < x2; i++, widthRatio += 1.0f/(h)) {
			int idx = j*bitmap.width + i;

			int spriteX = widthRatio * sprite.width;
			int spriteY = heightRatio * sprite.height;
			int spriteIdx = spriteY*sprite.width + spriteX;

			pixels[idx] = spritePixels[spriteIdx];
		}
	}
}

void DrawText(BitmapData bitmap, char* text, int size, int x, int y) {
	
}

void Render(BitmapData frameBuffer) {
	DrawBox(frameBuffer, 30, 50, 430, 220, RGBA(240, 180, 10, 255));
	DrawBox(frameBuffer, 40, 100, 50, 30, RGBA(20, 160, 220, 255));
	DrawBox(frameBuffer, 100, 430, 210, 90, RGBA(20, 80, 70, 255));
	DrawBox(frameBuffer, 220, 80, 630, 110, RGBA(20, 160, 70, 255));
	DrawBox(frameBuffer, 240, 230, 430, 70, RGBA(120, 60, 70, 255));
	DrawBitmap(frameBuffer, 700, 300, 200, 160, frameBuffer);
}

BitmapData LoadBMPFile(char* fileName) {
	FILE* bmpFile = fopen(fileName, "rb");
	
	if(bmpFile == NULL){
		printf("\n\nError: could not open file '%s'.\n", fileName);
		return {};
	}
	
	fseek(bmpFile, 0, SEEK_END);
	size_t fileSize = ftell(bmpFile);
	fseek(bmpFile, 0 ,SEEK_SET);
	
	unsigned char* fileBuffer = (unsigned char*)malloc(fileSize);
	fread(fileBuffer, fileSize, 1, bmpFile);
	fclose(bmpFile);
	
	BitMapHeader* bmpInfo = (BitMapHeader*)fileBuffer;
	int width  = bmpInfo->imageWidth;
	int height = bmpInfo->imageHeight;
	int* data = (int*)malloc(4*width*height);
	
	unsigned char* fileCursor = fileBuffer + bmpInfo->imageDataOffset;
	
	for(int j = 0; j < height; j++){
		for(int i = 0; i < width; i++){
			int pixel = fileCursor[0] | (fileCursor[1] << 8) | (fileCursor[2] << 16);
			fileCursor += 3;
			data[j*width+i] = pixel;
		}
	}
	
	free(fileBuffer);
	return {data, width, height};
}