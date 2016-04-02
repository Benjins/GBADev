#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

char* ReadTextFile(const char* fileName, int* outLength){
	FILE* file = fopen(fileName, "rb");
	
	if(file == 0){
		*outLength = 0;
		return 0;
	}
	
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	char* fileBuffer = (char*)malloc(fileSize+1);
	fread(fileBuffer, 1, fileSize, file);
	fclose(file);
	
	fileBuffer[fileSize] = '\0';
	
	*outLength = fileSize;
	return fileBuffer;
}

typedef unsigned short rgb15;

static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

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
	int* data;
	int width;
	int height;
} BitmapData;

BitmapData LoadBMPFile(const char* fileName) {
	FILE* bmpFile = fopen(fileName, "rb");
	
	if(bmpFile == NULL){
		printf("\n\nError: could not open file '%s'.\n", fileName);
		BitmapData err = {0};
		return err;
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
	
	BitmapData result = {data, width, height};
	free(fileBuffer);
	return result;
}

BitmapData ResizeBMP(BitmapData img, int width, int height){
	BitmapData newDat;
	newDat.width = width;
	newDat.height = height;
	newDat.data = (int*)malloc(newDat.width*newDat.height*4);
	
	float widthRatio = ((float)img.width)/width;
	float heightRatio = ((float)img.height)/height;
	
	float xVal=0,yVal=0;
	
	for(int j = 0; j < newDat.height; j++){
		for(int i = 0; i < newDat.width; i++){
			int newDatIdx = (j*newDat.width)+i;
			int imgIdx = ((int)yVal)*img.width + ((int)xVal);
			
			newDat.data[newDatIdx] = img.data[imgIdx];
			
			xVal += widthRatio;
		}
		xVal = 0;
		yVal += heightRatio;
	}
	
	return newDat;
}

rgb15* ResampleBMPTo16Bit(BitmapData data){
	rgb15* resample = (rgb15*)malloc(data.width*data.height*sizeof(rgb15));
	
	for(int i = 0; i < data.width*data.height; i++){
		int r = (data.data[i] & 0xFF0000) >> 16;
		int g = (data.data[i] & 0xFF00) >> 8;
		int b = (data.data[i] & 0xFF);
		
		resample[i] = RGB15(r >> 3, g >> 3, b >> 3);
	}
	
	return resample;
}

void WriteBitmapToAssetHeader(char* varName, int frameNum, BitmapData bmpData, FILE* assetHeaderFile){
	BitmapData resized = ResizeBMP(bmpData, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	rgb15* resampled = ResampleBMPTo16Bit(resized);
	
	fprintf(assetHeaderFile, "const uint16 %s%d[] __attribute__((section(\".rodata\")))= {\n", varName, frameNum);
	
	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
		int x = i % SCREEN_WIDTH;
		int y = i / SCREEN_WIDTH;
		
		int newY = SCREEN_HEIGHT - y - 1;
		int newIdx = x + newY*SCREEN_WIDTH;
		
		fprintf(assetHeaderFile, "%d,", resampled[newIdx]);
		if(i % 30 == 0){
			fprintf(assetHeaderFile, "\n");
		}
	}
	
	fprintf(assetHeaderFile, "\n};\n");
}

int main(int argc, char** argv){
	
	if (argc < 2){
		printf("Usage: %s [project directory]\n", argv[0]);
		return 0;
	}
	
	char vidFileName[256] = {0};
	snprintf(vidFileName, 256, "%s/vid.txt", argv[1]);
	
	int fileLength;
	char* fileBuffer = ReadTextFile(vidFileName, &fileLength);
	
	if(fileBuffer == NULL){
		printf("Could not find '%s', skipping.\n", vidFileName);
		return 0;
	}
	
	char vidName[256] = {0};
	char vidFileFormat[256] = {0};
	int vidStart = 0;
	int vidEnd = 0;
	sscanf(fileBuffer, "%s %s %d %d", vidName, vidFileFormat, &vidStart, &vidEnd);
	
	char vidAssetFileName[256] = {0};
	snprintf(vidAssetFileName, 256, "%s/vid.h", argv[1]);
	
	FILE* assetHeaderFile = fopen(vidAssetFileName, "wb");
	
	fprintf(assetHeaderFile, "typedef struct{const uint16** frames; int frameCount; int width; int height;} Video;\n");
	
	for(int i = vidStart; i <= vidEnd; i++){
		char frameFileName[256] = {0};
		snprintf(frameFileName, 256, vidFileFormat, i);
		
		char frameFullFileName[256] = {0};
		snprintf(frameFullFileName, 256, "%s/%s", argv[1], frameFileName);
		
		BitmapData frameBMP = LoadBMPFile(frameFullFileName);
		WriteBitmapToAssetHeader(vidName, i, frameBMP, assetHeaderFile);
	}
	
	
	fprintf(assetHeaderFile, "const uint16* %s_frames[] = {\n", vidName);
	for(int i = vidStart; i <= vidEnd; i++){
		fprintf(assetHeaderFile, "\t%s%d,\n", vidName, i);
	}
	fprintf(assetHeaderFile, "};");
	
	fprintf(assetHeaderFile, "Video %s = {%s_frames, %d, %d, %d};\n", vidName, vidName, vidEnd - vidStart + 1, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	fclose(assetHeaderFile);
	
	return 0;
}

