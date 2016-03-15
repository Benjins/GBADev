#include <stdio.h>
#include <time.h>

#include "token.h"

typedef enum{
	STRING,
	CHARACTER,
	INTEGER,
	FLOAT,
	OCTAL,
	HEX,
	OPERATOR,
	IDENTIFIER,
	LINECOMMENT,
	ANNOTATION_STR,
	ANNOTATION_ID,
	ANNOTATION_WS,
	BLOCKCOMMENT,
	WHITESPACE
} ParseState;

#define COMPARE_STRING(ptr, reference) (_memcmp(ptr, reference, (sizeof(reference) - 1)) == 0)
#define COMPARE_STRING_N(ptr, reference, len) (_memcmp(ptr, reference, MIN(sizeof(reference) - 1, len)) == 0)

#define TOKEN_IS(token, str) (COMPARE_STRING_N(token.start, str, token.length) && token.length == (sizeof(str)-1))

TokenVector LexString(const char* string){
	static const char* operators[] = {",", "++", "--", "*", "/", "->", "+", "-", "&", "|", "&&", "||", "#", "%", "{", "}", ";", "(", ")"
									 "~", "^", "!=", "==", "=", "!", ".", "?", ":", "<", ">", "<<", ">>", "<=", ">=", "[", "]", "+=",
									 "-=", "*=", "/=", "^=", "|=", "&=", "##"};
	
	static const Token annoStart = {"/*[", 3};
	static const Token annoEnd   = {"]*/", 3};
	
	TokenVector tokens = {0};
	
	ParseState currState = WHITESPACE;
	
	#define EMIT_TOKEN() {currToken.length = fileCursor - currToken.start+1;VectorAddToken(&tokens, currToken);\
						  currToken.length = 0; currToken.start = fileCursor+1;}
	
	int fileSize = _strlen(string);
	
	const char* fileCursor = string;
	Token currToken = {fileCursor, 0};
	
	while(fileCursor - string < fileSize){
		switch(currState){
			case STRING:{
				if(*fileCursor == '\\'){
					fileCursor++;
				}
				else if(*fileCursor == '"'){
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case CHARACTER:{
				if(*fileCursor == '\\'){
					fileCursor++;
				}
				else if(*fileCursor == '\''){
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case INTEGER:{
				if(*fileCursor == '.'){
					currState = FLOAT;
				}
				else if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case FLOAT:{
				if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case OCTAL:{
				if(*fileCursor < '0' || *fileCursor > '9'){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
				else if(*fileCursor == 'x' || *fileCursor == 'X'){
					currState = HEX;
				}
			}break;
			
			case HEX:{
				if((*fileCursor < '0' || *fileCursor > '9') && (*fileCursor < 'a' || *fileCursor > 'f')
					&& (*fileCursor < 'A' || *fileCursor > 'F')){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case OPERATOR:{
				bool found = false;

				for(int i = 0; i < ARRAY_COUNT(operators); i++){
					if(_memcmp(currToken.start, operators[i], fileCursor - currToken.start + 1) == 0){
						found = true;
						break;
					}
				}
				
				if(!found){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
			}break;
			
			case IDENTIFIER:{		
				if(FindChar(whitespace, *fileCursor) != -1){
					fileCursor--;
					EMIT_TOKEN();
					currState = WHITESPACE;
				}
				else{
					bool isOp = false;
					for(int  i = 0; i < ARRAY_COUNT(operators); i++){
						if(operators[i][0] == *fileCursor){
							isOp = true;
							break;
						}
					}
					
					if(isOp){
						fileCursor--;
						EMIT_TOKEN();
						currState = WHITESPACE;
					}
				}
			}break;
			
			case LINECOMMENT:{
				if(*fileCursor == '\n'){
					currState = WHITESPACE;
				}
				
				currToken.start++;
			}break;
			
			case ANNOTATION_STR:{
				if(*fileCursor == '"'){
					EMIT_TOKEN();
					break;
				}
			}
			case ANNOTATION_ID:{
				if(*fileCursor == ']'){
					fileCursor--;
					EMIT_TOKEN();
					VectorAddToken(&tokens, annoEnd);
					currState = BLOCKCOMMENT;
				}
				else if(FindChar(whitespace, *fileCursor) != -1 || *fileCursor == '('){
					fileCursor--;
					EMIT_TOKEN();
					currState = ANNOTATION_WS;
				}
			}break;

			case ANNOTATION_WS:{
				if(*fileCursor == ']'){
					VectorAddToken(&tokens, annoEnd);
					currState = BLOCKCOMMENT;
					break;
				}
				else if(*fileCursor == '(' || *fileCursor == ')'){
					Token thisTok = {fileCursor, 1};
					VectorAddToken(&tokens, thisTok);
					break;
				}
				else if(*fileCursor == '"'){
					currState = ANNOTATION_STR;
				}
				else if(FindChar(whitespace, *fileCursor) == -1){
					currState = ANNOTATION_ID;
				}
			}
			
			case BLOCKCOMMENT:{
				if(fileCursor - string < fileSize - 2 && fileCursor[0] == '*' && fileCursor[1] == '/'){
					currState = WHITESPACE;
					fileCursor += 2;
					currToken.start += 2;
				}
				else if(*fileCursor == '[' && *(fileCursor - 1) == '*' && *(fileCursor - 2) == '/'){
					VectorAddToken(&tokens, annoStart);
					currState = ANNOTATION_WS;
				}
				
				currToken.start++;
			}break;
			
			case WHITESPACE:{
				currToken.start = fileCursor;
				if(fileCursor - string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '/'){
					currState = LINECOMMENT;
				}
				else if(fileCursor - string < fileSize - 2 && fileCursor[0] == '/' && fileCursor[1] == '*'){
					currState = BLOCKCOMMENT;
				}
				else if(FindChar(whitespace, *fileCursor) == -1){
					
					if(*fileCursor == '"'){
						currState = STRING;
						fileCursor++;
					}
					else if(*fileCursor == '\''){
						currState = CHARACTER;
						fileCursor++;
					}
					else if(*fileCursor == '0'){
						currState = OCTAL;
					}
					else if(*fileCursor >= '1' && *fileCursor <= '9'){
						currState = INTEGER;
					}
					else{
						bool isOp = false;
						for(int i = 0; i < ARRAY_COUNT(operators); i++){
							if(operators[i][0] == *fileCursor){
								isOp = true;
								break;
							}
						}
						
						if(isOp){
							currState = OPERATOR;
						}
						else{
							currState = IDENTIFIER;
						}
					}
					
					fileCursor--;
				}
			}break;
		}
		
		fileCursor++;
	}
	
	if(currState != WHITESPACE){
		EMIT_TOKEN();
	}
	
	#undef EMIT_TOKEN
	
	return tokens;
}