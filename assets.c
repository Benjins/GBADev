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

#define MAX_BANK_COLOURS 16
#define MAX_BANKS_PALETTE 16

typedef struct{
	rgb15 cols[MAX_BANK_COLOURS];
	int size;
} PaletteBank;

typedef struct{
	PaletteBank banks[MAX_BANKS_PALETTE];
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


// Gah...
typedef struct{
	int* data;
	int width;
	int height;
} BitmapData;

BitmapData LoadBMPFile(char* fileName) {
	//So if it's null, we'll skip it
	//We're not using it, so just make it not-null but useless
	BitmapData dat = {(int*)0x01, 0 ,0};
	return dat;
}

#include "animer/AnimAsset.h"

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
	"typedef struct{int palIdx; int width; int height; unsigned short* data;} Sprite;\n "
	"typedef struct{Sprite map; Sprite* bgSprites; int* spriteFlags; int bgCount;} Background;\n";
	
	fprintf(assetsHeaderFile, "%s", headerStart);
	
	Palette palette;
	palette.size = 0;
	
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
			//printf("Found bg sprite named '%.*s'\n", spriteVarName.length, spriteVarName.start);
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
	
	char animFileName[256] = {};
	sprintf(animFileName, "%s/%s", argv[1], "anim.txt");
	FILE* animFile = fopen(animFileName, "rb");
	
	if(animFile != NULL){
		fclose(animFile);
		
		printf("Opening animation file '%s'\n", animFileName);
		
		fprintf(assetsHeaderFile, "typedef struct{Sprite* sprite; int duration;} AnimKey;\n");
		fprintf(assetsHeaderFile, "typedef struct{AnimKey* keys; int keyCount;} SpriteAnim;\n");
		
		AnimAsset animAsset = {};
		ReadAnimAssetFile(&animAsset, "anim.txt", argv[1], strlen(argv[1]));
		
		for(int i = 0; i < animAsset.animClipCount; i++){
			
			char* animName = animAsset.animClips[i].name;
			
			for(int j = 0; j < animAsset.animClips[i].keyFrameCount; j++){
				char varNameStr[256] = {};
				int varNameLength = sprintf(varNameStr, "%s_key%d_sprite", animName, j);
				Token varName = {varNameStr, varNameLength};
				
				Token fileName = {animAsset.animClips[i].keyFrames[j].fileName, strlen(animAsset.animClips[i].keyFrames[j].fileName)};
				
				WriteAsset(argv[1], varName, fileName, assetsHeaderFile, &palette, true);
				
				fprintf(assetsHeaderFile, "AnimKey %s_key%d = {&%s, %d};\n", animName, j, varNameStr, animAsset.animClips[i].keyFrames[j].duration);
			}
			
			fprintf(assetsHeaderFile, "AnimKey %s_keys[] = {\n", animName);
			for(int j = 0; j < animAsset.animClips[i].keyFrameCount; j++){
				fprintf(assetsHeaderFile, "%s_key%d,\n", animName, j);
			}
			fprintf(assetsHeaderFile, "};\n");
			
			fprintf(assetsHeaderFile, "SpriteAnim %s = { %s_keys, %d };", animName, animName, animAsset.animClips[i].keyFrameCount);
		}
		
		//WriteAsset(argv[1], varName, fileName, assetsHeaderFile, &palette, true);
	}
	else{
		printf("Could not find '%s', skipping.\n", backgroundFileName);
	}
	
	WritePalette(&palette, assetsHeaderFile);
	
	fclose(assetsHeaderFile);
	free(assetFileContents);
	
	return 0;
}

typedef struct {
	unsigned char Red;
	unsigned char Green;
	unsigned char Blue;
	unsigned char Alpha;
} RGBAPixel;

bool MergePaletteBanks(PaletteBank a, PaletteBank b, PaletteBank* out){
	PaletteBank possibleOut = a;
	
	for(int i = 0; i < b.size; i++){
		bool found = false;
		for(int j = 0; j < possibleOut.size; j++){
			if(b.cols[i] == possibleOut.cols[j]){
				found = true;
				break;
			}
		}
		
		if(!found){
			if(possibleOut.size >= 15){
				return false;
			}
			else{
				possibleOut.cols[possibleOut.size] = b.cols[i];
				possibleOut.size++;
			}
		}
	}
	
	*out = possibleOut;
	return true;
}

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
	
	//printf("width: %d, height: %d\n", width, height);

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
	
	fprintf(assetHeader, "static Sprite %.*s = {%d, %d, %d, %.*s_data\n};\n", varName.length, varName.start, 0, width, height, varName.length, varName.start);
	
	free(pixelData);
}

void WriteAsset(char* folderName, Token varName, Token fileName, FILE* assetHeader, Palette* palette, bool tileMemory){
	//printf("Found asset named '%.*s' at file '%.*s'.\n", varName.length, varName.start, fileName.length, fileName.start);
	
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
	
	//printf("width: %d, height: %d\n", width, height);

	RGBAPixel* pixelData = (RGBAPixel*)malloc(width*height*sizeof(RGBAPixel));
	
	for(int i = 0; i < width*height; i++){
		pixelData[i].Blue = imgBuffer[i*3];
		pixelData[i].Green = imgBuffer[i*3+1];
		pixelData[i].Red = imgBuffer[i*3+2];
		pixelData[i].Alpha = 255;
	}
	
	free(imgBuffer);
	
	unsigned char* indices = (unsigned char*) malloc(width*height);
	
	PaletteBank newBank = {};
	newBank.size = 1;
	newBank.cols[0] = 0;
	
	for(int j = 0; j < height; j++){
		for(int i = 0; i < width; i++){	
			int index = j*width+i;
			int memIdx = (height - 1 - j) * width + i;
			if(tileMemory){
				int realY = (height - 1 - j);
				int blockXIdx = i / 8;
				int blockYIdx = realY / 8;
				memIdx = blockYIdx * 64 * (width/8) + blockXIdx * 64 + (realY % 8) * 8 + (i % 8);
			}
			
			int paletteIndex = -1;
			rgb15 shiftedColor = RGB15(pixelData[index].Red / 8, pixelData[index].Green / 8, pixelData[index].Blue / 8);
			for(int pIdx = 0; pIdx < newBank.size; pIdx++){
				if(newBank.cols[pIdx] == shiftedColor){
					paletteIndex = pIdx;
					break;
				}
			}
			
			if(paletteIndex == -1){
				newBank.cols[newBank.size] = shiftedColor;
				paletteIndex = newBank.size;
				newBank.size++;
			}
			
			indices[memIdx] = paletteIndex;
		}
	}
	
	int palIdx = -1;
	for(int i = 0; i < palette->size; i++){
		PaletteBank mergeBank = {};
		if(MergePaletteBanks(palette->banks[i], newBank, &mergeBank)){
			palette->banks[i] = mergeBank;
			palIdx = i;
			break;
		}
	}
	
	if(palIdx == -1){
		palette->banks[palette->size] = newBank;
		palIdx = palette->size;
		palette->size++;
	}
	
	fprintf(assetHeader, "static unsigned short %.*s_data[] = {\n", varName.length, varName.start);
	for(int i = 0; i < width*height; i++){
		fprintf(assetHeader, "%d,", indices[i]);
	}
	fprintf(assetHeader, "};\n");
	
	fprintf(assetHeader, "static Sprite %.*s = {%d, %d, %d, %.*s_data\n};\n", varName.length, varName.start, palIdx, width, height, varName.length, varName.start);
	
	free(indices);
	
	free(pixelData);
}

void WritePalette(Palette* palette, FILE* assetHeader){
	fprintf(assetHeader, "\nrgb15 paletteColors[] = {\n\t");
	for(int i = 0; i < palette->size; i++){
		for(int j = 0; j < palette->banks[i].size; j++){
			fprintf(assetHeader, "%d,", palette->banks[i].cols[j]);
		}
		for(int j = palette->banks[i].size; j < 16; j++){
			fprintf(assetHeader, "0,");
		}
		printf("\n\t");
	}
	fprintf(assetHeader, "};\n");
}
