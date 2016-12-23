#if !defined(VEC_PARAM)
#error "Need to define VEC_PARAM before including vec.h"
#endif

#ifndef _GLUE_TOKENS
#define _GLUE_TOKENS(a, b) a ## b
#endif

#ifndef GLUE_TOKENS
#define GLUE_TOKENS(a, b) _GLUE_TOKENS(a,b)
#endif

#ifndef VEC_TYPE
#define VEC_TYPE GLUE_TOKENS(VEC_PARAM, Vector)
#endif

typedef struct {
	VEC_PARAM* data;
	int capacity;
	int count;
} VEC_TYPE;

#include <stdlib.h>

void* memcpy(void* p1, const void* p2, size_t bytes);
void* malloc(size_t bytes);
void free(void* ptr);

void GLUE_TOKENS(VEC_TYPE, EnsureCapacity) (VEC_TYPE* vec, int cap){
	int newCapacity = vec->capacity;
	while (newCapacity < cap){
		newCapacity = ((newCapacity == 0) ? 2 : (2 * newCapacity));
	}
	
	// If we need to re-allocate
	if (newCapacity > vec->capacity){
		VEC_PARAM* newData = (VEC_PARAM*)malloc(newCapacity * sizeof(VEC_PARAM));
		memcpy(newData, vec->data, vec->count * sizeof(VEC_PARAM));
		free(vec->data);
		vec->data = newData;
		vec->capacity = newCapacity;
	}
}

void GLUE_TOKENS(VEC_TYPE, PushBack) (VEC_TYPE* vec, VEC_PARAM val){
	GLUE_TOKENS(VEC_TYPE, EnsureCapacity) (vec, vec->count + 1);
	vec->data[vec->count] = val;
	vec->count++;
}

void GLUE_TOKENS(VEC_TYPE, Clear) (VEC_TYPE* vec){
	vec->count = 0;
}

void GLUE_TOKENS(VEC_TYPE, Destroy) (VEC_TYPE* vec){
	free(vec->data);
	vec->data = 0;
	vec->count = 0;
	vec->capacity = 0;
}

#if defined(VEC_PARAM)
#undef VEC_PARAM
#endif

#if defined(VEC_TYPE)
#undef VEC_TYPE
#endif

#if defined(_GLUE_TOKENS)
#undef _GLUE_TOKENS
#endif

#if defined(GLUE_TOKENS)
#undef GLUE_TOKENS
#endif
