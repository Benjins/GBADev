#ifndef TOKEN_H
#define TOKEN_H

#include "../toolsCode/util.h"

typedef struct {
	const char* start;
	int length;
} Token;

#define MAKE_TOKEN(str) { #str, (sizeof(#str)-1) }

int TokenEqual(Token a, Token b) {
	if (a.length == b.length) {
		return _memcmp(a.start, b.start, a.length) == 0;
	}

	return 0;
}

DEFINE_VECTOR(Token)

#endif