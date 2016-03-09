#include <stdio.h>

void* malloc(size_t size);

void free(void* ptr);

typedef unsigned char uint8, byte;
typedef char int8;

#define bool int
#define true 1
#define false 0

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef ARRAY_COUNT
#define ARRAY_COUNT(x) ((sizeof(x))/sizeof((x)[0]))
#endif

#define UNUSED(x) ((void)(x))

static const char* whitespace = "\n\r\t ";

int _memcmp(const void* p1, const void* p2, size_t len){
	const char* s1 = (const char*)p1;
	const char* s2 = (const char*)p2;
	
	if(s1 == 0 && s2 == 0){
		return 0;
	}
	else if(s1 != 0 && s2 != 0){
		while(*s1 != 0 && *s2 != 0 && *s1 ==*s2 && len > 1){
			s1++;
			s2++;
			len--;
		}
		
		return (*s2 - *s1);
	}
	else{
		return (s1 == 0 ? -1 : 1);
	}
}

void _memcpy(void* _dest, const void* _src, int len){
	char* dest = (char*)_dest;
	const char* src  = (const char*)_src;
	
	for(int i = 0; i < len; i++){
		dest[i] = src[i];
	}
}

void _memcpyBack(void* _dest, const void* _src, int len){
	char* dest = (char*)_dest;
	const char* src  = (const char*)_src;
	
	for(int i = len - 1; i >= 0; i--){
		dest[i] = src[i];
	}
}

int FindChar(const char* _str, char c){
	if(!_str){
		return -1;
	}
	
	for(const char* str = _str; *str; str++){
		if(*str == c){
			return str - _str;
		}
	}

	return -1;
}

byte* ReadBinaryFile(const char* fileName, int* outLength){
	FILE* file = fopen(fileName, "rb");
	
	if(file == 0){
		*outLength = 0;
		return 0;
	}
	
	fseek(file, 0, SEEK_END);
	int fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	byte* fileBuffer = (byte*)malloc(fileSize);
	fread(fileBuffer, 1, fileSize, file);
	fclose(file);
	
	*outLength = fileSize;
	return fileBuffer;
}

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

int _strlen(const char* str){
	int len = 0;
	while(str && *str){
		len++;
		str++;
	}
	
	return len;
}

int _atoi(const char* str){
	const char* cursor = str;
	while(cursor && *cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r'){
		cursor++;
	}
	
	int sign = 1;
	if(cursor && *cursor == '-'){
		sign = -1;
		cursor++;
	}
	
	int value = 0;
	while(cursor && *cursor > '0' && *cursor < '9'){
		value *= 10;
		value += (*cursor - '0');
		cursor++;
	}
		
	return value*sign;
}

const int vectorCapacityStep = 50;

#define DEFINE_VECTOR(Type) \
typedef struct{             \
	Type* vals;             \
	int length;             \
	int capacity;           \
} Type##Vector;             \
                            \
void VectorEnsureCapacity##Type(Type##Vector* vec, int capacity){\
	int newCapacity = ((capacity + vectorCapacityStep-1)/vectorCapacityStep) * vectorCapacityStep;\
	if(vec->capacity < newCapacity){\
		Type* newVals = (Type*)malloc(newCapacity*sizeof(Type));\
		_memcpy(newVals, vec->vals, sizeof(Type)*vec->length);\
		free(vec->vals);\
		vec->vals = newVals;\
		vec->capacity = newCapacity;\
	}\
}\
void VectorAdd##Type (Type##Vector* vec, Type newVal){\
	VectorEnsureCapacity##Type(vec, vec->length+1);\
	vec->vals[vec->length] = newVal;\
	vec->length++;\
}   \
void VectorReset##Type (Type##Vector* vec){ \
	vec->length = 0; \
	vec->capacity = 0;\
	free(vec->vals); \
} \
void VectorResetWithoutFree##Type (Type##Vector* vec){ \
	vec->length = 0; \
	vec->capacity = 0;\
	vec->vals = (Type*)0; \
} \
bool VectorSafeGet##Type (Type##Vector* vec, int index, Type* outVal){ \
	if(index >= 0 && index < vec->length){\
		*outVal = vec->vals[index];\
		return true; \
	}\
	return false;\
} \
void VectorInsert##Type (Type##Vector* vec, int index, Type val){ \
	if(index >= 0 && index < vec->length){ \
		VectorEnsureCapacity##Type(vec, vec->length + 1);\
		for(int i = vec->length; i > index; i--){ \
			vec->vals[i] = vec->vals[i-i]; \
		}\
		vec->vals[index] = val; \
		vec->length++;\
	}\
} \
void VectorInsertVector##Type (Type##Vector* vec, int index, Type##Vector vecAdd){ \
	if(index >= 0 && index <= vec->length){ \
		VectorEnsureCapacity##Type(vec, vec->length + vecAdd.length);\
		for(int i = vec->length; i >= index; i--){\
			vec->vals[i+vecAdd.length] = vec->vals[i];\
		}\
		_memcpy(&vec->vals[index], vecAdd.vals, vecAdd.length*sizeof(Type)); \
		vec->length += vecAdd.length;\
	}\
} \
void VectorRemove##Type (Type##Vector* vec, int index){ \
	if(index >= 0 && index < vec->length){ \
		for(int i = index; i < vec->length - 1; i++){ \
			vec->vals[i] = vec->vals[i+1]; \
		}\
		vec->length--; \
	}\
}

typedef char* CStr;
typedef const char* ConstCStr;

DEFINE_VECTOR(int)
DEFINE_VECTOR(float)
DEFINE_VECTOR(CStr)
DEFINE_VECTOR(ConstCStr)

typedef struct{
	char* start;
	char* cursor;
	int capacity;
} StrBuffer;


void EnsureCapacity(StrBuffer* buf, int capacity){
	if(capacity > buf->capacity){
		int bytesWritten = buf->cursor - buf->start;
		
		char* newBuffer = (char*)malloc(capacity+1);
		_memcpy(newBuffer, buf->start, bytesWritten);
		newBuffer[bytesWritten] = '\0';
		free(buf->start);
		buf->start = newBuffer;
		buf->cursor = newBuffer + bytesWritten;
		buf->capacity = capacity;
	}
}

void EnsureAdditionalCapacity(StrBuffer* buf, int additional){
	EnsureCapacity(buf, (int)(buf->cursor - buf->start) + additional);
}

void AddNToBuffer(StrBuffer* buf, const char* str, int len){
	EnsureAdditionalCapacity(buf, len);
	_memcpy(buf->cursor, str, len);
	buf->cursor[len] = '\0';
	buf->cursor += len;
}

void AddToBuffer(StrBuffer* buf, const char* str){
	AddNToBuffer(buf, str, _strlen(str));
}

void InsertNIntoBuffer(StrBuffer* buf, int index, const char* str, int len){
	int bufferSize = buf->cursor - buf->start;
	if(index < 0 || index >= bufferSize){
		printf("\n\nError, inserting at index %d into buffer of size %d\n", index, bufferSize);
		return;
	}
	
	EnsureAdditionalCapacity(buf, len);
	_memcpyBack(buf->start + index + len, buf->start + index, bufferSize - index);
	_memcpy(buf->start + index, str, len);
	buf->cursor[len] = '\0';
	buf->cursor += len;
}

void InsertIntoBuffer(StrBuffer* buf, int index, const char* str){
	InsertNIntoBuffer(buf, index, str, _strlen(str));
}

void RemoveFromBuffer(StrBuffer* buf, int index, int count){
	int bufferSize = buf->cursor - buf->start;
	if(index < 0 || (index + count) > bufferSize){
		printf("\n\nError, Removing %d at index %d into buffer of size %d\n", count, index, bufferSize);
		return;
	}
	
	_memcpy(buf->start + index, buf->start + index + count, bufferSize - index);
	buf->cursor -= count;
	buf->cursor[0] = '\0';
}

void ResetBuffer(StrBuffer* buf){
	buf->cursor = buf->start;
	if(buf->cursor){
		buf->cursor[0] = '\0';
	}
}

void FreeBuffer(StrBuffer* buf){
	ResetBuffer(buf);
	free(buf->start);
	buf->capacity = 0;
}