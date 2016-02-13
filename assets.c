#include <stdio.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef char int8;
typedef short int16;
typedef int int32;
typedef int bool;

#define true 1
#define false 0

#define TILE_INDEX_MULTIPLIER 24

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

typedef struct{
	char* start;
	int length;
}Token;

typedef uint16 rgb15;

static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

typedef struct{
	rgb15 data[256];
	int size;
} Palette;

//I don't like including header files for just a few functions.
char* strcat(char* dest, char* src);

char* strchr(char* str, int character);

size_t strcspn(const char* str1, const char* str2);

size_t strspn(const char* str1, const char* str2);

size_t strlen(const char * str);

char* strstr(char* s1, const char* s2);

int sprintf (char* str, const char* format, ... );

void* malloc(size_t size);

void free(void* ptr);

void* memcpy(void* destination, const void* source, size_t num);

void WriteAsset(char* folderName, Token varName, Token fileName, FILE* assetHeader, Palette* palette, bool tileMemory);

void WritePalette(Palette* palette, FILE* assetHeader);

void WriteBackground(char* folderName, Token varName, Token fileName, FILE* assetHeader);

#include "tileMapper/BackgroundParsing.h"

int main(int argc, char** argv){

	if(argc != 2){
		printf("Usage: assets.exe [name of folder]\n");
		return -1;
	}
	
	char assetFileName[256] = {};
	strcat(assetFileName, argv[1]);
	strcat(assetFileName, "/assets.txt");
	FILE* assetsFile = fopen(assetFileName, "rb");
	
	if(assetsFile == NULL){
		printf("Warning, could not find file '%s'.\n", assetFileName);
		return -1;
	}
	
	char assetHeaderFileName[256] = {};
	strcat(assetHeaderFileName, argv[1]);
	strcat(assetHeaderFileName, "/assets.h");
	FILE* assetsHeaderFile = fopen(assetHeaderFileName, "wb");
	
	if(assetsHeaderFile == NULL){
		printf("Warning, could not find file '%s'.\n", assetHeaderFileName);
		return -1;
	}
	
	fseek(assetsFile, 0, SEEK_END);
	size_t assetFileSize = ftell(assetsFile);
	fseek(assetsFile, 0, SEEK_SET);
	
	char* assetFileContents = (char*)malloc(assetFileSize+1);
	fread(assetFileContents, 1, assetFileSize, assetsFile);
	fclose(assetsFile);
	
	assetFileContents[assetFileSize] = '\0';
	
	char* cursor = assetFileContents;
	
	static const char* whitespace = "\n\r\t ";
	
	char headerStart[512] = 
	"typedef struct{int type; int width; int height; unsigned short* data;} Sprite;\n \
	 typedef struct{Sprite map; Sprite* bgSprites; int* spriteFlags; int bgCount;} Background;\n";
	
	fprintf(assetsHeaderFile, "%s", headerStart);
	
	Palette palette;
	palette.data[0] = 0;
	palette.size = 1;
	
	while(cursor != NULL && cursor >= assetFileContents && (cursor - assetFileContents) < assetFileSize){
		cursor += strspn(cursor, whitespace);
		
		if(*cursor == '\0'){
			break;
		}
		
		char* colon = strchr(cursor, ':');
		
		if(*colon == '\0'){
			break;
		}
		
		Token varName;
		varName.start = cursor;
		varName.length = colon - cursor;
		
		cursor = colon + 1;
		cursor += strspn(cursor, whitespace);
		
		Token fileName;
		fileName.start = cursor;
		fileName.length = strcspn(cursor, whitespace);
		cursor += fileName.length;
		
		WriteAsset(argv[1], varName, fileName, assetsHeaderFile, &palette, true);
	}
	
	char backgroundFileName[256] = {};
	strcat(backgroundFileName, argv[1]);
	strcat(backgroundFileName, "/background.txt");
	FILE* backgroundFile = fopen(backgroundFileName, "rb");
	if(backgroundFile != NULL){
		printf("Opening background file '%s'.\n", backgroundFileName);
		
		fclose(backgroundFile);
		BackgroundAsset bgAsset = ParseBGAssetFile(backgroundFileName);
		
		for(int i = 0; i < bgAsset.spriteCount; i++){
			char spriteVarNameStr[256] = {};
			int spriteVarNameLength = sprintf(spriteVarNameStr, "%s_bg%d", bgAsset.backMap, i);
			Token spriteVarName = {spriteVarNameStr, spriteVarNameLength};
			printf("Found bg sprite named '%.*s'\n", spriteVarName.length, spriteVarName.start);
			Token bgSpriteFile = {bgAsset.sprites[i].fileName, strlen(bgAsset.sprites[i].fileName)};
			WriteAsset(argv[1], spriteVarName, bgSpriteFile, assetsHeaderFile, &palette, true);
		}
		
		char bgVarNameStr[256] = {};
		int bgVarNameLength = sprintf(bgVarNameStr, "%s_map", bgAsset.backMap);
		Token bgVarName = {bgVarNameStr, bgVarNameLength};
		
		Token bgFileName = {bgAsset.backMapFile, strlen(bgAsset.backMapFile)};
		WriteBackground(argv[1], bgVarName, bgFileName, assetsHeaderFile);
		
		fprintf(assetsHeaderFile, "Sprite %s_bgs[] = {\n", bgAsset.backMap);
		for(int i = 0; i < bgAsset.spriteCount; i++){
			fprintf(assetsHeaderFile, "%s_bg%d,\n", bgAsset.backMap, i);
		}		
		fprintf(assetsHeaderFile, "};\n");
		
		fprintf(assetsHeaderFile, "int %s_flags[] = {\n", bgAsset.backMap);
		for(int i = 0; i < bgAsset.spriteCount; i++){
			fprintf(assetsHeaderFile, "%d,", bgAsset.sprites[i].flags);
		}
		fprintf(assetsHeaderFile, "};\n");
		
		fprintf(assetsHeaderFile, "Background %s = {%s_map, %s_bgs, %s_flags, %d};\n", 
				bgAsset.backMap, bgAsset.backMap, bgAsset.backMap, bgAsset.backMap, bgAsset.spriteCount);
	}
	else{
		printf("Could not find '%s', skipping.\n", backgroundFileName);
	}
	
	WritePalette(&palette, assetsHeaderFile);
	
	fclose(assetsHeaderFile);
	free(assetFileContents);

	printf("Done packing assets for '%s'\n", argv[1]);
	
	return 0;
}

typedef struct {
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;
} RGBAPixel;

void WriteBackground(char* folderName, Token varName, Token fileName, FILE* assetHeader){
	int folderNameLength = strlen(folderName);
	char* fileNameCpy = (char*)malloc(folderNameLength + 1 + fileName.length+1);
	memcpy(fileNameCpy, folderName, folderNameLength);
	fileNameCpy[folderNameLength] = '/';
	memcpy(fileNameCpy+folderNameLength+1, fileName.start, fileName.length);
	fileNameCpy[folderNameLength + 1 + fileName.length] = '\0';
	
	FILE* bmpFile = fopen(fileNameCpy, "rb");
	if(bmpFile == NULL){
		printf("Error, could not open bmp file '%s'.\n", fileNameCpy);
		return;
	}
	
	BitMapHeader header;	
	fread(&header, sizeof(header), 1, bmpFile);

	int imageDataSize = header.imageDataSize;
	if(imageDataSize == 0){
		imageDataSize = header.imageHeight * header.imageWidth * header.bitDepth / 8;
	}

	fseek(bmpFile, header.imageDataOffset - sizeof(header), SEEK_CUR);
	
	unsigned char* imgBuffer = (unsigned char*)malloc(imageDataSize);
	fread(imgBuffer, 1, imageDataSize, bmpFile);
	fclose(bmpFile);

	int width = header.imageWidth;
	int height = header.imageHeight;
	
	printf("width: %d, height: %d\n", width, height);

	int* pixelData = (int*)malloc(width*height*sizeof(int));
	
	for(int i = 0; i < width*height; i++){
		pixelData[i]  = imgBuffer[i*3];
		pixelData[i] |= imgBuffer[i*3+1] << 8;
		pixelData[i] |= imgBuffer[i*3+2] << 16;
	}
	
	free(imgBuffer);
	
	fprintf(assetHeader, "static unsigned short %.*s_data[] = {\n", varName.length, varName.start);
	for(int j = 0; j < height; j++){
		for(int i = 0; i < width; i++){
			int idx = (height - 1 - j) * width + i;
			fprintf(assetHeader, "%d,", pixelData[idx] / TILE_INDEX_MULTIPLIER);
		}
	}
	fprintf(assetHeader, "};\n");
	
	int type = 0;
	if(width == 32 && height == 8){
		type = 0x4000;
	}
	
	fprintf(assetHeader, "static Sprite %.*s = {%d, %d, %d, %.*s_data\n};\n", varName.length, varName.start, type, width, height, varName.length, varName.start);
	
	free(pixelData);
}

void WriteAsset(char* folderName, Token varName, Token fileName, FILE* assetHeader, Palette* palette, bool tileMemory){
	printf("Found asset named '%.*s' at file '%.*s'.\n", varName.length, varName.start, fileName.length, fileName.start);
	
	int folderNameLength = strlen(folderName);
	char* fileNameCpy = (char*)malloc(folderNameLength + 1 + fileName.length+1);
	memcpy(fileNameCpy, folderName, folderNameLength);
	fileNameCpy[folderNameLength] = '/';
	memcpy(fileNameCpy+folderNameLength+1, fileName.start, fileName.length);
	fileNameCpy[folderNameLength + 1 + fileName.length] = '\0';
	
	FILE* bmpFile = fopen(fileNameCpy, "rb");
	if(bmpFile == NULL){
		printf("Error, could not open bmp file '%s'.\n", fileNameCpy);
		return;
	}
	
	BitMapHeader header;	
	fread(&header, sizeof(header), 1, bmpFile);

	int imageDataSize = header.imageDataSize;
	if(imageDataSize == 0){
		imageDataSize = header.imageHeight * header.imageWidth * header.bitDepth / 8;

	}
	fseek(bmpFile, header.imageDataOffset - sizeof(header), SEEK_CUR);

	unsigned char* imgBuffer = (unsigned char*)malloc(imageDataSize);
	fread(imgBuffer, 1, imageDataSize, bmpFile);
	fclose(bmpFile);

	int width = header.imageWidth;
	int height = header.imageHeight;
	
	printf("width: %d, height: %d\n", width, height);

	RGBAPixel* pixelData = (RGBAPixel*)malloc(width*height*sizeof(RGBAPixel));
	
	for(int i = 0; i < width*height; i++){
		pixelData[i].Blue = imgBuffer[i*3];
		pixelData[i].Green = imgBuffer[i*3+1];
		pixelData[i].Red = imgBuffer[i*3+2];
		pixelData[i].Alpha = 255;
	}
	
	free(imgBuffer);
	
	unsigned char* indices = (unsigned char*) malloc(width*height);
	
	for(int j = 0; j < height; j++){
		for(int i = 0; i < width; i++){	
			int index = j*width+i;
			int memIdx = (height - 1 - j) * width + i;
			if(tileMemory){
				int blockIdx = i / 8;
				memIdx = blockIdx * 64 + (height - 1 - j) * 8 + (i % 8);
			}
			
			int paletteIndex = -1;
			rgb15 shiftedColor = RGB15(pixelData[index].Red / 8, pixelData[index].Green / 8, pixelData[index].Blue / 8);
			for(int pIdx = 0; pIdx < palette->size; pIdx++){
				if(palette->data[pIdx] == shiftedColor){
					//printf("Set pltIdx to '%d'\n", pIdx);
					paletteIndex = pIdx;
					break;
				}
			}
			
			if(paletteIndex == -1){
				palette->data[palette->size] = shiftedColor;
				paletteIndex = palette->size;
				palette->size++;
				//printf("Increment palette->size to '%d'\n", palette->size);
			}
			
			indices[memIdx] = paletteIndex;
		}
	}
	
	fprintf(assetHeader, "static unsigned short %.*s_data[] = {\n", varName.length, varName.start);
	for(int i = 0; i < width*height; i++){
		fprintf(assetHeader, "%d,", indices[i]);
	}
	fprintf(assetHeader, "};\n");
	
	int type = 0;
	if(width == 32 && height == 8){
		type = 0x4000;
	}
	
	fprintf(assetHeader, "static Sprite %.*s = {%d, %d, %d, %.*s_data\n};\n", varName.length, varName.start, type, width, height, varName.length, varName.start);
	
	free(indices);
	
	free(pixelData);
}

void WritePalette(Palette* palette, FILE* assetHeader){
	fprintf(assetHeader, "rgb15 paletteColors[] = {\n");
	for(int i = 0; i < palette->size; i++){
		fprintf(assetHeader, "%d,", palette->data[i]);
	}
	fprintf(assetHeader, "};\n");
}