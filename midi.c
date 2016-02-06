#include <stdio.h>

typedef unsigned char byte;

void* malloc(size_t size);

void free(void* ptr);

typedef enum{
	SINGLE_TRACK,
	SIMULTANEOUS_MULTITRACK,
	SEQUENTIAL_MULTIRACK
} TrackFormat;

typedef struct{
	short format;
	short trackCount;
	short deltaTimeDivision;
} MidiHeader;

typedef enum{
	NOTE_OFF,
	NOTE_ON,
	POLYPHONIC_PRESSURE,
	CONTROL_CHANGE,
	PROGRAM_CHANGE,
	CHANNEL_PRESSURE,
	PITCH_WHEEL_CHANGE,
	CHANNEL_MSG_TYPE_COUNT
} MidiChannelMsgType;

typedef struct{
	int note;
	int start;
	int length;
} Note;

int channelMsgLengths[CHANNEL_MSG_TYPE_COUNT] = {2,2,2,2,1,1,2};

int lengthContaining(const char* str, const char* key);

int lengthNotContaining(const char* str, const char* key);

#define bool int
#define true 1
#define false 0

int _memcpy(void* dest, const void* src, int length){
	byte* _dest = (byte*)dest;
	byte* _src  = (byte*)src;
	
	for(int i = 0; i < length; i++){
		_dest[i] = _src[i];
	}
}

int _memcmp(void* p1, void* p2, int len){
	byte* b1 = (byte*)p1;
	byte* b2 = (byte*)p2;
	
	for(int i = 0; i < len; i++){
		if(b1[i] < b2[i]){
			return 1;
		}
		else if(b1[i] > b2[i]){
			return -1;
		}
	}
	
	return 0;
}

int GetVariableLength(byte* stream, int* outByteCount){
	int bytes = 0;
	int total = 0;
	while((stream[bytes] & 0x80) != 0){
		total = total * 128;
		total = total + (stream[bytes] & 0x7F);
		bytes++;
	}
	
	total = total * 128;
	total = total + (stream[bytes] & ~0x80);
	bytes++;
	
	*outByteCount = bytes;
	
	return total;
}

int SwapEndianness4(int val){
	return ((val & 0xFF) << 24)    | ((val & 0xFF00) << 8)
	   	 | ((val & 0xFF0000) >> 8) | ((val & 0xFF000000) >> 24);
}

short SwapEndianness2(short val){
	return ((val & 0xFF) << 8) | ((val & 0xFF00) >> 8);
}

#define ASSERT(x) {if(!(x)){printf("Assertion failed!\n'%s'\n", #x);}}

void ParseMidiFile(const char* fileName, char* varName, FILE* midiAssetHeader);

int main(int argc, char** argv){
	
	if(argc < 2){
		printf("Usage: midi.exe [name of directory]\n");
		return 0;
	}
	
	const char* dirName = argv[1];

	char midiTxtFileName[256] = {};
	sprintf(midiTxtFileName, "%s/midi.txt", dirName);
	
	FILE* midiTextFile = fopen(midiTxtFileName, "rb");
	
	if(midiTextFile == NULL){
		printf("Could not open '%s', skipping.\n", midiTxtFileName);
		return 0;
	}
	
	fseek(midiTextFile, 0, SEEK_END);
	int midiTextFileSize = ftell(midiTextFile);
	fseek(midiTextFile, 0, SEEK_SET);
	
	char* midiTextFileBuffer = (char*)malloc(midiTextFileSize + 1);
	fread(midiTextFileBuffer, 1, midiTextFileSize, midiTextFile);
	midiTextFileBuffer[midiTextFileSize] = '\0';
	
	fclose(midiTextFile);
	
	char midiAssetFileName[256] = {};
	sprintf(midiAssetFileName, "%s/midi.h", dirName);
	FILE* midiAssetFile = fopen(midiAssetFileName, "wb");
	
	fprintf(midiAssetFile, "typedef struct{int start; short length; short pitch;} Note;");
	fprintf(midiAssetFile, "typedef struct{Note* notes; int length;} Song;");
	
	char* fileCursor = midiTextFileBuffer;
	while(fileCursor - midiTextFileBuffer < midiTextFileSize){
		fileCursor += lengthContaining(fileCursor, "\t\r\n ");
		int midiFileNameLength = lengthNotContaining(fileCursor, "\t\r\n :");
		
		char* midiFileName = (char*)malloc(midiFileNameLength+1);
		_memcpy(midiFileName, fileCursor, midiFileNameLength);
		midiFileName[midiFileNameLength] = '\0';
		
		fileCursor += midiFileNameLength;
		fileCursor += lengthContaining(fileCursor, "\t\r\n ");
		ASSERT(*fileCursor == ':');
		fileCursor++;
		fileCursor += lengthContaining(fileCursor, "\t\r\n ");
		
		int varNameLength = lengthNotContaining(fileCursor, "\t\r\n ");
		char* midiVarName = (char*)malloc(varNameLength+1);
		_memcpy(midiVarName, fileCursor, varNameLength);
		midiVarName[varNameLength] = '\0';
		
		char midiFileNameFull[256] = {};
		sprintf(midiFileNameFull, "%s/%s", dirName, midiFileName);
		
		ParseMidiFile(midiFileNameFull, midiVarName, midiAssetFile);
		
		fileCursor += varNameLength;
		fileCursor += lengthContaining(fileCursor, "\t\r\n ");
		
		free(midiFileName);
		free(midiVarName);
	}
	
	fclose(midiAssetFile);
	
	return 0;
}

void ParseMidiFile(const char* fileName, char* varName, FILE* midiAssetHeader){
	FILE* midiFile = fopen(fileName, "rb");
	
	if(midiFile == NULL){
		printf("Error, could not open file '%s'.\n", fileName);
		return;
	}
	
	fseek(midiFile, 0, SEEK_END);
	int fileSize = ftell(midiFile);
	fseek(midiFile, 0, SEEK_SET);
	
	byte* fileBuffer = (byte*)malloc(fileSize);
	
	fread(fileBuffer, 1, fileSize, midiFile);
	
	byte* fileCursor = fileBuffer;
	
	MidiHeader header = {0};
	
	byte test1 = 0;
	byte test2 = 0x40;
	byte test3 = 0x7F;
	short test4 = 0x0081;
	short test5 = 0x00C0;
	short test6 = 0x7FFF;
	int test7 = 0x00808081;
	int test8 = 0x008080C0;
	
	int byteCount;
	
	ASSERT(GetVariableLength((byte*)&test1, &byteCount) == 0);
	ASSERT(GetVariableLength((byte*)&test2, &byteCount) == 0x40);
	ASSERT(GetVariableLength((byte*)&test3, &byteCount) == 0x7F);
	ASSERT(GetVariableLength((byte*)&test4, &byteCount) == 0x80);
	ASSERT(GetVariableLength((byte*)&test5, &byteCount) == 0x2000);
	ASSERT(GetVariableLength((byte*)&test6, &byteCount) == 0x3FFF);
	ASSERT(GetVariableLength((byte*)&test7, &byteCount) == 0x0200000);
	ASSERT(GetVariableLength((byte*)&test8, &byteCount) == 0x8000000);
	
	Note allNotes[1024] = {};
	int allNoteCount = 0;
	
	Note currNotes[12] = {};
	int currNoteCount = 0;
	
	int currTime = 0;
	
	int microSecsPerQuarterNote = 500000;

	while(fileCursor - fileBuffer < fileSize){
		char chunkType[5] = {fileCursor[0], fileCursor[1], fileCursor[2], fileCursor[3], 0};
		
		fileCursor += 4;
		
		int chunkLength = SwapEndianness4(*(int*)fileCursor);
		
		printf("Found chunk of type '%s' and length '%d'\n", chunkType, chunkLength);
		
		fileCursor += 4;
		
		if(_memcmp(chunkType, "MThd", 4) == 0){
			header = *(MidiHeader*)fileCursor;
			header.format = SwapEndianness2(header.format);
			header.trackCount = SwapEndianness2(header.trackCount);
			header.deltaTimeDivision = SwapEndianness2(header.deltaTimeDivision);
			printf("header.format = '%d', trackCnt = '%d', div = '%d'\n", header.format, header.trackCount, header.deltaTimeDivision);
			
			fileCursor += chunkLength;
		}
		else if(_memcmp(chunkType, "MTrk", 4) == 0){
			printf("Foum dtravlk.\n");
			
			byte* chunkStart = fileCursor;
			
			byte prevEventType = 0;
			
			byte prevStatus = 0;
			while(fileCursor - chunkStart < chunkLength){
				int varBytes = 0;
				int deltaTime = GetVariableLength(fileCursor, &varBytes);
				
				//printf("deltaTime: %d\n", deltaTime);
				
				for(int i = 0; i < currNoteCount; i++){
					currNotes[i].length += deltaTime;
				}
				
				currTime += deltaTime;
				
				fileCursor += varBytes;
				
				byte eventType = *fileCursor;
				
				fileCursor++;
				
				if(eventType == 0xFF){
					byte metaEventType = *fileCursor;
					fileCursor++;					
					
					byte eventLength = *fileCursor;
					fileCursor++;
					
					printf("Meta event (0x%02X) (%3d): ", metaEventType, eventLength);
					
					if(metaEventType == 0x51){
						ASSERT(eventLength <= 3);
						
						microSecsPerQuarterNote = 0;
						for(int i = 0; i < eventLength; i++){
							microSecsPerQuarterNote *= 256;
							microSecsPerQuarterNote += fileCursor[i];
						}
					}
					
					for(int i = 0; i < eventLength; i++){
						printf("0x%0X ", fileCursor[i]);
					}
					printf("\n");
						
					fileCursor += eventLength;
					
				}
				else if(eventType == 0xF7){
					byte eventLength = *fileCursor;
					fileCursor++;
						
					fileCursor += eventLength;
				}
				else if(eventType == 0xF0){
					byte eventLength = *fileCursor;
					fileCursor++;
					
					
					fileCursor += eventLength;
				}
				else{
					if((eventType & 0x80) == 0){
						eventType = prevStatus;
						fileCursor--;
					}
					
					int channel = eventType & 0x0F;
					int msgType = (eventType & 0x70) >> 4;
					
					if(msgType == NOTE_ON){
						currNotes[currNoteCount].note = fileCursor[0];
						currNotes[currNoteCount].length = 0;
						currNotes[currNoteCount].start = currTime;
						currNoteCount++;
						//printf("Note on: %d\n", fileCursor[0]);
					}
					else if(msgType == NOTE_OFF){
						for(int i = 0; i < currNoteCount; i++){
							if(currNotes[i].note == fileCursor[0]){
								allNotes[allNoteCount] = currNotes[i];
								allNoteCount++;
								currNotes[i] = currNotes[currNoteCount-1];
								currNoteCount--;
								break;
							}
						}
						//printf("Note off: %d\n", fileCursor[0]);
					}
					
					fileCursor += channelMsgLengths[msgType];
						
				}	
				
				prevStatus = eventType;				
			}
			
			
			
			//printf("fileCursor: 0x%012X, chunkEnd: 0x%012X\n", fileCursor, chunkStart + chunkLength);
			ASSERT(fileCursor == chunkStart + chunkLength);
		}		
	}
	
	printf("currNoteCount: %d\n", currNoteCount);
	ASSERT(currNoteCount == 0);
	
	fprintf(midiAssetHeader, "Note %s_notes[] = {\n", varName);
	
	for(int i = 0; i < allNoteCount; i++){
		int ticksPerQuarter = header.deltaTimeDivision;
		int noteTicks = allNotes[i].length;
		int samplesPerClock = 96;
		int clocksPerSecond = 60;
		
		long cycleLength = (long)noteTicks * (long)microSecsPerQuarterNote * 60 * 96;
		cycleLength /= ticksPerQuarter;
		cycleLength /= 1000000;
		
		long cycleStart = (long)(allNotes[i].start) * (long)microSecsPerQuarterNote * 60 * 96;
		cycleStart /= ticksPerQuarter;
		cycleStart /= 1000000;
		
		int pitchIdx = (allNotes[i].note % 12);
		
		short pitches[12] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};
		
		short pitch = (96*60)/pitches[pitchIdx];
		
		printf("Note '%d' at '%d' for length '%d'\n", pitch, cycleStart, cycleLength);
		
		fprintf(midiAssetHeader, "{%d, %d, %d}, ", cycleStart, cycleLength, pitch);
		
		if(i % 5 == 4){
			fprintf(midiAssetHeader, "\n");
		}
		
	}
	
	fprintf(midiAssetHeader, "};\n");
	
	fprintf(midiAssetHeader, "Song %s = {%s_notes, %d};", varName, varName, allNoteCount);
	
	free(fileBuffer);
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