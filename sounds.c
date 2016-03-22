#include <stdio.h>

#define bool int
#define true 1
#define false 0

typedef unsigned char byte;

void* malloc(size_t size);

void free(void* ptr);

void ParseSoundFile(char* sndFileName, FILE* soundHeader);

int lengthContaining(const char* str, const char* key);

int lengthNotContaining(const char* str, const char* key);

int _memcmp(const void* p1, const void* p2, int length);

void _memcpy(void* dest, const void* src, int length);

byte* ReadEntireFile(char* fileName, int* outFileLength, bool writeZero){
	FILE* soundsFile = fopen(fileName, "rb");
	
	if(soundsFile == NULL){
		printf("Error: Could not open file '%s'.\n", fileName);
		*outFileLength = 0;
		return NULL;
	}
	
	fseek(soundsFile, 0, SEEK_END);
	*outFileLength = ftell(soundsFile);
	fseek(soundsFile, 0, SEEK_SET);
	
	byte* fileBuffer = (byte*)malloc(*outFileLength + (writeZero ? 1 : 0));
	
	fread(fileBuffer, 1, *outFileLength, soundsFile);
	
	if(writeZero){
		fileBuffer[*outFileLength] = '\0';
	}
	
	fclose(soundsFile);
	
	return fileBuffer;
}

int main(int argc, char** argv){

	if(argc < 2){
		printf("Usage: sounds.exe [folder name].\n");
		return 0;
	}

	char* dirName = argv[1];
	
	char soundsFileName[256] = {};
	sprintf(soundsFileName, "%s/sounds.txt", dirName);
	
	int fileLength = 0;
	char* fileBuffer = (char*)ReadEntireFile(soundsFileName, &fileLength, true);
	
	if(fileBuffer == NULL){
		return 0;
	}
	
	printf("Read in assets descr file.\n");
	
	int length = lengthNotContaining(fileBuffer, "\n\t\r ");

	printf("About to parse sound file.\n");
	
	char wavFileName[256] = {};
	sprintf(wavFileName, "%s/%.*s", dirName, length, fileBuffer);
	
	char soundHeaderFileName[256] = {};
	sprintf(soundHeaderFileName, "%s/sounds.h", dirName);
	FILE* soundHeader = fopen(soundHeaderFileName, "wb");
	
	fprintf(soundHeader, "typedef struct{ const unsigned char* data; int dataLength; } SoundClip;\n");
	
	ParseSoundFile(wavFileName, soundHeader);
	
	fclose(soundHeader);
	
	printf("Parsed sound file.\n");
	
	free(fileBuffer);
	
	return 0;
}

int lengthContaining(const char* str, const char* key){
	const char* cursor = str;
	while(*cursor){
		
		bool found = false;
		for(int i = 0; key[i]; i++){
			if(key[i] == *cursor){
				found = true;
				break;
			}
		}
		
		if(!found){
			break;
		}
		
		cursor++;
	}
	
	return cursor - str;
}

int lengthNotContaining(const char* str, const char* key){
	const char* cursor = str;
	while(*cursor){
		bool found = false;
		for(int i = 0; key[i]; i++){
			if(key[i] == *cursor){
				found = true;
				break;
			}
		}
		
		if(found){
			break;
		}
		
		cursor++;
	}
	
	return cursor - str;
}

typedef struct{
	short AudioFormat;  
	short NumChannels;  
	int SampleRate;
	int ByteRate;     
	short BlockAlign;   	
	short BitsPerSample;
} WaveFormatHeader;

void WriteSoundHeader(byte* data, int dataLen, WaveFormatHeader hdr, FILE* soundHeader, int newSampleRate){
	double audioLengthInSeconds = ((double)dataLen)/hdr.ByteRate;
	
	int newSampleCount = (int)(audioLengthInSeconds*newSampleRate);
	
	printf("Resampled sound to %d samples.\n", newSampleCount);
	
	byte* newSamples = (byte*)malloc(newSampleCount);
	
	int dataIdx = 0;
	double deltaErr = ((double)hdr.SampleRate)/newSampleRate;
	double error = 0;
	for(int i = 0; i < newSampleCount; i++){
		double newSample = 0;
		
		error += deltaErr;
		int resampleCount = 0;
		while(error >= 1){
			double currDataSample = (data[dataIdx]);
			
			if(hdr.BitsPerSample == 16){
				currDataSample = *(short*)(&data[dataIdx]);
			}
			
			currDataSample /= (1 << (hdr.BitsPerSample-8));
			newSample += currDataSample;
			dataIdx += (hdr.BitsPerSample/8);
			
			error -= 1.0;
			resampleCount++;
		}
		
		newSamples[i] = (byte)(newSample/resampleCount);
	}
	
	fprintf(soundHeader, "const unsigned char snd_data[] __attribute__((section(\".rodata\"))) = {\n");
	for(int i = 0; i < newSampleCount; i++){
		fprintf(soundHeader, "%d, ", newSamples[i]);
		if(i % 20 == 19){
			fprintf(soundHeader, "\n");
		}
	}
	
	fprintf(soundHeader, "};\n");
	fprintf(soundHeader, "SoundClip snd = {snd_data, %d};\n", newSampleCount);
	
	free(newSamples);
}

void ParseSoundFile(char* sndFileName, FILE* soundHeader){
	int fileLength = 0;
	byte* fileBuffer = ReadEntireFile(sndFileName, &fileLength, false);
	
	if(fileBuffer == NULL){
		return;
	}
	
	printf("Parsing sound file: '%s'\n", sndFileName);
	
	byte* fileCursor = fileBuffer;
	
	WaveFormatHeader format = {};
	byte* data = NULL;
	int dataSize = 0;
	
	if(_memcmp(fileCursor, "RIFF", 4) == 0){
		fileCursor += 8;
		
		if(_memcmp(fileCursor, "WAVE", 4) == 0){
			fileCursor += 4;
			while(fileCursor - fileBuffer < fileLength){
				if(_memcmp(fileCursor, "fmt ", 4) == 0){
					fileCursor += 4;
					int chunkSize = *(int*)fileCursor;
					fileCursor += 4;
					
					format = *(WaveFormatHeader*)fileCursor;
					
					printf("Found format: fmt: %d, numChannels: %d, SampleRate: %d,\n ByteRate: %d, blockAlgn: %d, bitsPerSample: %d\n",
							format.AudioFormat, format.NumChannels, format.SampleRate, format.ByteRate, format.BlockAlign, format.BitsPerSample);
					
					fileCursor += chunkSize;
				}
				else if(_memcmp(fileCursor, "data", 4) == 0){
					fileCursor += 4;
					int chunkSize = *(int*)fileCursor;
					fileCursor += 4;
					
					byte* oldData = data;
					data = malloc(dataSize+chunkSize);
					
					_memcpy(data, oldData, dataSize);
					_memcpy(data+dataSize, fileCursor, chunkSize);
					
					dataSize += chunkSize;
					
					free(oldData);
					
					printf("Found data of %d bytes, starts with: |%0X, %0X, %0X|.\n", chunkSize, data[0], data[1], data[2]);
					
					fileCursor += chunkSize;
				}
				else if(_memcmp(fileCursor, "LIST", 4) == 0){
					fileCursor += 4;
					int chunkSize = *(int*)fileCursor;
					fileCursor += 4;
					
					fileCursor += chunkSize;
				}
				else if(_memcmp(fileCursor, "id3 ", 4) == 0){
					fileCursor += 4;
					int chunkSize = *(int*)fileCursor;
					fileCursor += 4;
					
					fileCursor += chunkSize;
				}
				else{
					printf("Error in file '%s', unexpected chunk ID: '%.*s'\n", sndFileName, 4, fileCursor);
					break;
				}
			}
		}
		else{
			printf("Error, '%s' may not be a proper WAVE file, header unexpected format.\n", sndFileName);
		}
	}
	else{
		printf("Error, '%s' may not be a proper WAVE file, missing header.\n", sndFileName);
	}
	
	const int newSampleRate = 18157;
	
	WriteSoundHeader(data, dataSize, format, soundHeader, newSampleRate);
	
	free(data);
	free(fileBuffer);
}

int _memcmp(const void* p1, const void* p2, int length){
	byte* s1 = (byte*)p1;
	byte* s2 = (byte*)p2;
	
	for(int i = 0; i < length; i++){
		if(s1[i] < s2[i]){
			return -1;
		}
		else if(s1[i] > s2[i]){
			return 1;
		}
	}
	
	return 0;
}

void _memcpy(void* dest, const void* src, int length){
	byte* _dest = (byte*)dest;
	byte* _src  = (byte*)src;
	
	for(int i = 0; i < length; i++){
		_dest[i] = _src[i];
	}
}