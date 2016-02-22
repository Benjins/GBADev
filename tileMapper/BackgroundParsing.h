typedef enum {
	NONE = 0,
	WALKABLE = (1 << 0),
	MAX_SPRITE_FLAG = (1 << 1)
} BGSpriteFlags;

typedef struct{
	char* fileName;
	BGSpriteFlags flags;
} BGSprite;

typedef struct {
	char* backMap;
	char* backMapFile;
	BGSprite* sprites;
	int spriteCount;
} BackgroundAsset;

void WriteBGAssetFile(BackgroundAsset bgAsset, char* fileName) {
#if defined(_MSC_VER)
	FILE* bgFile = NULL;
	fopen_s(&bgFile, fileName, "wb");
#else
	FILE* bgFile = fopen(fileName, "wb");
#endif

	if (bgFile == NULL) {
		printf("Error, could not open file '%s'\n", fileName);
	}
	else{
		fprintf(bgFile, "%s:%s:", bgAsset.backMap, bgAsset.backMapFile);
		
		for(int i = 0; i < bgAsset.spriteCount; i++){
			fprintf(bgFile, "%s%s;%d", (i == 0 ? "" : ","), bgAsset.sprites[i].fileName, bgAsset.sprites[i].flags);
		}
		
		fprintf(bgFile, "\n");
	}
}

int atoi(const char* str);

BackgroundAsset ParseBGAssetFile(char* fileName) {
#if defined(_MSC_VER)
	FILE* bgFile = NULL;
	fopen_s(&bgFile, fileName, "rb");
#else
	FILE* bgFile = fopen(fileName, "rb");
#endif

	const char* whitespace = "\n\r\t ";
	const char* spaceAndColon = "\t :";

	if (bgFile != NULL) {
		fseek(bgFile, 0, SEEK_END);
		size_t fileSize = ftell(bgFile);
		fseek(bgFile, 0, SEEK_SET);

		char* fileBuffer = (char*)malloc(fileSize + 1);
		fread(fileBuffer, 1, fileSize, bgFile);
		fileBuffer[fileSize] = '\0';
		fclose(bgFile);

		BackgroundAsset bgAsset = {};

		char* backMapStart = fileBuffer + strspn(fileBuffer, whitespace);
		int backMapLength = strcspn(backMapStart, spaceAndColon);

		bgAsset.backMap = (char*)malloc(backMapLength+1);
		memcpy(bgAsset.backMap, backMapStart, backMapLength);
		bgAsset.backMap[backMapLength] = '\0';

		char* backMapEnd = backMapStart + backMapLength;
		char* backMapFileStart = backMapEnd + strspn(backMapEnd, spaceAndColon);
		int backMapFileLength = strcspn(backMapFileStart, spaceAndColon);

		bgAsset.backMapFile = (char*)malloc(backMapFileLength + 1);
		memcpy(bgAsset.backMapFile, backMapFileStart, backMapFileLength);
		bgAsset.backMapFile[backMapFileLength] = '\0';
		
		char* bgCursor = backMapFileStart + backMapFileLength + 1;
		char* nextComma = bgCursor;
		char* nextNewline = strstr(bgCursor, "\n");
		while (nextComma != NULL && (nextComma < nextNewline || nextNewline == NULL)) {
			bgCursor += strspn(bgCursor, whitespace);

			char* endOfFileName = bgCursor + strcspn(bgCursor, " \t\n\r;,");
			Token spriteFileName;
			spriteFileName.start = bgCursor;
			spriteFileName.length = endOfFileName - bgCursor;
			
			char* startOfFlags = endOfFileName + strspn(endOfFileName, "\t\n\r ;");
			char* endOfFlags = startOfFlags + strspn(startOfFlags, "0123456789");
			int spriteFlagsVal = 0;
			if(endOfFlags != startOfFlags){
				spriteFlagsVal = atoi(startOfFlags);
			}
			

			BGSprite* newSprites = (BGSprite*)malloc((bgAsset.spriteCount+1)*sizeof(BGSprite));
			memcpy(newSprites, bgAsset.sprites, bgAsset.spriteCount*sizeof(BGSprite));

			newSprites[bgAsset.spriteCount].fileName = (char*)malloc(spriteFileName.length+1);
			memcpy(newSprites[bgAsset.spriteCount].fileName, spriteFileName.start, spriteFileName.length);
			newSprites[bgAsset.spriteCount].fileName[spriteFileName.length] = '\0';
			newSprites[bgAsset.spriteCount].flags = (BGSpriteFlags)spriteFlagsVal;
			free(bgAsset.sprites);
			bgAsset.sprites = newSprites;

			nextComma = strstr(bgCursor, ",");
			nextNewline = strstr(bgCursor, "\n");
			bgCursor = nextComma + 1;
			bgAsset.spriteCount++;
		}

		free(fileBuffer);

		return bgAsset;
	}
	else {
		printf("Could not open bg file '%s'.", fileName);
		BackgroundAsset none = {};
		return none;
	}
}