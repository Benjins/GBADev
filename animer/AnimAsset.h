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

void RemoveAnimKeyFrame(AnimClip* clip, int index){
	if(index > 0 && index < clip->keyFrameCount){
		free(clip->keyFrames[index].spriteData.data);
		free(clip->keyFrames[index].fileName);
		
		for(int i = index + 1; i < clip->keyFrameCount; i++){
			clip->keyFrames[i-1] = clip->keyFrames[i];
		}
		
		clip->keyFrameCount--;
	}
}

void AddKeyFrame(AnimClip* clip, AnimKeyFrame keyFrame){
	AnimKeyFrame* newKeyFrames = (AnimKeyFrame*)malloc(sizeof(AnimKeyFrame)*(clip->keyFrameCount+1));
	memcpy(newKeyFrames, clip->keyFrames, clip->keyFrameCount*sizeof(AnimKeyFrame));
	newKeyFrames[clip->keyFrameCount] = keyFrame;
	
	free(clip->keyFrames);
	clip->keyFrames = newKeyFrames;
	clip->keyFrameCount++;
}

void NoramlizeAnimClip(AnimClip* clip){
	//Skip the first two, its duration should never be negative and it messes up the logic
	for(int i = 1; i < clip->keyFrameCount - 1; i++){
		if(clip->keyFrames[i].duration < 0){
			clip->keyFrames[i-1].duration = clip->keyFrames[i].duration + clip->keyFrames[i-1].duration;
			clip->keyFrames[i+1].duration   = clip->keyFrames[i].duration + clip->keyFrames[i+1].duration;
			clip->keyFrames[i].duration *= -1;
			
			AnimKeyFrame temp = clip->keyFrames[i];
			clip->keyFrames[i] = clip->keyFrames[i+1];
			clip->keyFrames[i+1] = temp;
		}
	}
}

void ReadAnimAssetFile(AnimAsset* asset, const char* fileName, const char* dirName, int dirLength){
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

void SaveAnimAssetFile(AnimAsset* asset, const char* fileName){
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