#include "Renderer.h"

#include <stdio.h>
#include <stdlib.h>

extern int currMouseX;
extern int currMouseY;
extern int mouseState;

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

#define STB_TRUETYPE_IMPLEMENTATION  // force following include to generate implementation
#include "stb_truetype.h"

unsigned char tempBitmap[512*512];
BitmapData fontBMP = {};

stbtt_bakedchar fontBakeData[96]; // ASCII 32..126 is 95 glyphs

void InitText(const char* fileName, int size){
	FILE* fontFile = fopen(fileName, "rb");
	
	fseek(fontFile, 0, SEEK_END);
	int fileSize = ftell(fontFile);
	fseek(fontFile, 0, SEEK_SET);
	
	unsigned char* fontFileBuffer = (unsigned char*)malloc(fileSize);
	fread(fontFileBuffer, 1, fileSize, fontFile);
	
	int offsetInFile = stbtt_GetFontOffsetForIndex(fontFileBuffer, 0);
	stbtt_BakeFontBitmap(fontFileBuffer, offsetInFile, size, tempBitmap, 512, 512, 32, 96, fontBakeData);
	
	fontBMP.width = 512;
	fontBMP.height = 512;
	fontBMP.data = (int*)malloc(512*512*sizeof(int));
	for(int i = 0; i < 512*512; i++){
		int c = tempBitmap[i];
		fontBMP.data[i] = c | (c << 8) | (c << 16) | (0xFF << 24);
	}
}

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

bool Button(BitmapData frameBuffer, int x, int y, int w, int h, int offCol, int hoverCol, int pressCol, char* label){
	int mouseIsOver = 0;
	if(currMouseX > x && currMouseX < x + w
	&& currMouseY > y && currMouseY < y + h){
		mouseIsOver = 1;
	}
	
	int fillCol = offCol;
	if(mouseIsOver){
		fillCol = hoverCol;
		
		if(mouseState == 3){
			fillCol = pressCol;
		}
	}
	
	for(int j = y; j < y + h; j++){
		for(int i = x; i < x + w; i++){
			int frameIdx = (frameBuffer.height - 1 - j)*frameBuffer.width+i;
			frameBuffer.data[frameIdx] = fillCol;
		}
	}
	
	return mouseIsOver && (mouseState == 1);
}

void DrawText(BitmapData bitmap, char* text, int x, int y, int width, int height) {
	int currX = x;
	int currY = y;
	
	char* currChar = text;
	while(currChar && *currChar){
		
		int fontIdx = *currChar - 32;
		int w = fontBakeData[fontIdx].x1 - fontBakeData[fontIdx].x0; 
		int h = fontBakeData[fontIdx].y1 - fontBakeData[fontIdx].y0; 
		int fontStartX = fontBakeData[fontIdx].x0;
		int fontStartY = fontBakeData[fontIdx].y0;
		
		int xAdv = (int)fontBakeData[fontIdx].xadvance;
		
		int yOffset = (int)fontBakeData[fontIdx].yoff;
		
		for(int j = 0; j < h; j++){
			for(int i = 0; i < w; i++){
				int fontX = fontStartX + i;
				int fontY = (fontStartY + j);
				int fontIdx = fontX + fontY*fontBMP.width;
				
				int bmpX = currX + i;
				int bmpY = bitmap.height - 1 - (currY + j + yOffset);
				int bmpIdx = bmpX + bmpY*bitmap.width;
				
				int alpha = fontBMP.data[fontIdx] & 0xFF;
				for(int i = 0; i < 3; i++){
					int mask = (0x000000FF << i*8);
					int old = (bitmap.data[bmpIdx] & mask) >> (i*8);
					old = (old * (255-alpha))/255 + alpha;
					bitmap.data[bmpIdx] = (bitmap.data[bmpIdx] & ~mask) | (old&0xFF) << (i*8);
				}
			}
		}
		
		currX += xAdv;
		
		if(currX > bitmap.width || currX > x + width){
			currX = x;
			currY += (h - yOffset);
		}
		
		if(currY > bitmap.height - 1 - h || currY > y + height){
			break;
		}
		
		
		currChar++;
	}
}

void Render(BitmapData frameBuffer) {

}

BitmapData LoadBMPFile(const char* fileName) {
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

void WriteBMPFile(const char* fileName, BitmapData * bmp){
	FILE* bmpFile = fopen(fileName, "wb");

	if (bmpFile == NULL) {
		printf("\n\nError: could not open file '%s' for writing.\n", fileName);
		return;
	}

	int imageDataSize = 3 * bmp->width*bmp->height;

	BitMapHeader header = {};
	header.bitDepth = 24;
	header.imageWidth = bmp->width;
	header.imageHeight = bmp->height;
	header.imageDataOffset = sizeof(header);
	header.headerSize = 40;
	header.fileTag = 0x4D42;
	header.fileSize = sizeof(header) + imageDataSize;
	header.numColorPlanes = 1;

	fwrite(&header, 1, sizeof(header), bmpFile);
	
	unsigned char* dataBuffer = (unsigned char*)malloc(imageDataSize);

	int* bmpDataCast = (int*)bmp->data;

	for (int i = 0; i < bmp->width * bmp->height; i++) {
		dataBuffer[3 * i]     = (bmpDataCast[i] & 0xFF);
		dataBuffer[3 * i + 1] = (bmpDataCast[i] & 0xFF00) >> 8;
		dataBuffer[3 * i + 2] = (bmpDataCast[i] & 0xFF0000) >> 16;
	}

	fwrite(dataBuffer, 1, imageDataSize, bmpFile);

	fclose(bmpFile);

	free(dataBuffer);
}
