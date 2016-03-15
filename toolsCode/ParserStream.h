#ifndef PARSERSTREAM_H
#define PARSERSTREAM_H

#include <stdio.h>

#include "util.h"

void* malloc(size_t);
void free(void*);

int _memeq(const void* a, const void* b, int len){
	const char* s1 = (const char*)a;
	const char* s2 = (const char*)b;
	
	for(int i = 0; i < len; i++){
		if(s1[i] != s2[i]){
			return 0;
		}
	}
	
	return 1;
}

int _streq(const char* s1, const char* s2){
	while(*s1 && *s2){
		if(*s1 != *s2){
			return 0;
		}
		
		s1++;
		s2++;
	}
	
	return (*s1 == *s2) ? 1 : 0;
}

int FindStr(const char* haystack, const char* needle){
	const char* hCursor = haystack;
	const char* nCursor = needle;
	
	while(*nCursor && *hCursor){
		if(*nCursor != *hCursor){
			hCursor = hCursor - (nCursor - needle) + 1;
			nCursor = needle;
		}
		else{
			nCursor++;
			hCursor++;
		}
	}
	
	if(*hCursor == '\0' && *nCursor != '\0'){
		return -1;
	}
	else{
		return (hCursor - haystack) - (nCursor - needle);
	}
}

int FindSubStr(const char* haystack, const char* needle, int length){
	const char* hCursor = haystack;
	const char* nCursor = needle;
	
	while(nCursor - needle < length && *hCursor){
		if(*nCursor != *hCursor){
			hCursor = hCursor - (nCursor - needle) + 1;
			nCursor = needle;
		}
		else{
			nCursor++;
			hCursor++;
		}
	}
	
	if(*hCursor == '\0' && *nCursor != '\0'){
		return -1;
	}
	else{
		return (hCursor - haystack) - (nCursor - needle);
	}
}

char* AllocateStringFromToken(Token tok){
	char* str = (char*)malloc(tok.length+1);
	_memcpy(str, tok.start, tok.length);
	str[tok.length] = '\0';
	
	return str;
}

int Tokenize(char* source, int length, const char* delim, Token* outTokens, int allowEmpty){
	char* lastTokenEnd = source;
	int tokenCount = 0;
	int delimLength = _strlen(delim);
	
	while(lastTokenEnd - source < length){
		int nextOccurence = FindStr(lastTokenEnd, delim);
		if(nextOccurence == -1){
			int tokenLen = length - (lastTokenEnd - source);
			Token newTok = {lastTokenEnd, tokenLen};
			outTokens[tokenCount] = newTok;
			tokenCount++;
			lastTokenEnd = source + length;
		}
		else if(allowEmpty || nextOccurence != 0){
			Token newTok = {lastTokenEnd, nextOccurence};
			outTokens[tokenCount] = newTok;
			tokenCount++;
			lastTokenEnd = lastTokenEnd + nextOccurence + delimLength;
		}
		else{
			lastTokenEnd = lastTokenEnd + delimLength;
		}
	}
	
	return tokenCount;
}

typedef struct{
	char* start;
	char* cursor;
	int length;
}ParserStream;

ParserStream CreateParserStream(char* start, int length){
	ParserStream stream = {start, start, length};
	
	return stream;
}

int LengthLeft(ParserStream* stream){
	return stream->length - (stream->cursor - stream->start);
}

Token AdvanceToString(ParserStream* stream, const char* str){
	int len = FindStr(stream->cursor, str);
	if(len == -1 || len > LengthLeft(stream)){
		len = LengthLeft(stream);
	}
	
	Token tok = {stream->cursor, len};
	stream->cursor += len;
	return tok;
}

Token AdvanceToToken(ParserStream* stream, Token token){
	int len = FindSubStr(stream->cursor, token.start, token.length);
	if(len == -1 || len > LengthLeft(stream)){
		len = LengthLeft(stream);
	}
	
	Token tok = {stream->cursor, len};
	stream->cursor += len;
	return tok;
}

Token AdvanceUntilOneOf(ParserStream* stream, const char* str){
	int amt = 0;
	int lengthLeft = LengthLeft(stream);
	while(amt < lengthLeft){
		if(FindChar(str, stream->cursor[amt]) != -1){
			break;
		}
		amt++;
	}
	
	Token tok = {stream->cursor, amt};
	stream->cursor += amt;
	return tok;
}

Token AdvanceUntilNotOneOf(ParserStream* stream, const char* str){
	int amt = 0;
	int lengthLeft = LengthLeft(stream);
	while(amt < lengthLeft){
		if(FindChar(str, stream->cursor[amt]) == -1){
			break;
		}
		amt++;
	}
	
	Token tok = {stream->cursor, amt};
	stream->cursor += amt;
	return tok;
}

Token AdvancePast(ParserStream* stream, const char* str){
	Token advance = AdvanceToString(stream, str);
	int strLen = _strlen(str);
	if(strLen > LengthLeft(stream)){
		strLen = LengthLeft(stream);
	}
	
	advance.length += strLen;
	stream->cursor += strLen;
	return advance;
}

Token AdvanceLength(ParserStream* stream, int length){
	int lengthLeft = LengthLeft(stream);
	
	if(length > lengthLeft){
		length = lengthLeft;
	}
	
	Token tok = {stream->cursor, length};
	stream->cursor += length;
	return tok;
}

Token TrimToken(Token tok, const char* toTrim) {
	while (tok.length > 0 && FindChar(toTrim, *tok.start)) {
		tok.start++;
		tok.length--;
	}

	while (tok.length > 0 && FindChar(toTrim, tok.start[tok.length-1])) {
		tok.length--;
	}

	return tok;
}

#endif