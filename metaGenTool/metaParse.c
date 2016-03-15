#include "metaParse.h"

#include "preprocess.c"


int FindAttribute(AttributeUseVector attrs, Token name){
	
	for(int i = 0; i < attrs.length; i++){
		if(TokenEqual(attrs.vals[i].attrName, name)){
			return i;
		}
	}
	
	return -1;
}

int FindToken(TokenVector vec, Token tok){
	for(int i = 0; i < vec.length; i++){
		if(TokenEqual(vec.vals[i], tok)){
			return i;
		}
	}
	
	return -1;
}

int ParseAttributes(TokenVector tokens, int startIdx, AttributeUseVector* attrs){
	//printf("Parse attributes: startIdx (%d)\n", startIdx);
	int i = startIdx;
	while(i < tokens.length){
		if(TOKEN_IS(tokens.vals[i], "/*[")){
			AttributeUse attr = {0};
			attr.attrName = tokens.vals[i+1];
			//printf("attr found w/ name: '%.*s'\n", attr.attrName.length, attr.attrName.start);
			while(i < tokens.length && !TOKEN_IS(tokens.vals[i], "]*/")){
				if(tokens.vals[i].start[0] == '"'){
					VectorAddToken(&attr.attrParams, tokens.vals[i]);
				}
				i++;
			}
			
			VectorAddAttributeUse(attrs, attr);
		}
		else{
			break;
		}
		i++;
	}
	
	return i;
}

int ParseFields(TokenVector tokens, TokenVector* knownTypes, int startIdx, StructDef* def){
	AttributeUseVector attrs = {0};
	int i = startIdx;
	for( ; i < tokens.length && !TOKEN_IS(tokens.vals[i], "}"); i++){
		i = ParseAttributes(tokens, i, &attrs);
		int idx = FindToken(*knownTypes, tokens.vals[i]);
		 if(idx != -1){
			 
			 int ptrCount = 0;
			 Token nextTok = {0};
			 i++;
			 
			 while(VectorSafeGetToken(&tokens, i, &nextTok) && TOKEN_IS(nextTok, "*")){
				 ptrCount++;
				 i++;
			 }
			 
			 Token fieldName = nextTok;
			 Token arrayCount = {0};
			 
			 Token tokAfterField = {0};
			 if(VectorSafeGetToken(&tokens, i+1, &tokAfterField) && TOKEN_IS(tokAfterField, "[")){
				Token otherBracket = {0};
				if(VectorSafeGetToken(&tokens, i+3, &otherBracket) && TOKEN_IS(otherBracket, "]")){
					arrayCount = tokens.vals[i+2];
				}
			 }
			 
			 FieldDef fieldDef = {0};
			 fieldDef.typeName = knownTypes->vals[idx];
			 fieldDef.fieldName = fieldName;
			 fieldDef.pointerLevel = ptrCount;
			 fieldDef.attrs = attrs;
			 attrs.length = 0;
			 attrs.capacity = 0;
			 attrs.vals = NULL;
			 
			 if(arrayCount.length == 0){
				fieldDef.arrayCount = NOT_AN_ARRAY;
			 }
			 else{
				fieldDef.arrayCount = _atoi(arrayCount.start);
			 }
			 
			 VectorAddFieldDef(&def->fields, fieldDef);
			
			/*
			 printf("       Field of type '%.*s', name '%.*s', ptrCount: '%d', arrCount: %.*s\n", 
						knownTypes->vals[idx].length, knownTypes->vals[idx].start,
						fieldName.length, fieldName.start,
						ptrCount,
						arrayCount.length, arrayCount.start);
			*/
		 }
	}
	
	return i;
}

StructDefVector ParseTokensForStructs(TokenVector tokens, TokenVector* knownTypes){
	StructDefVector structDefs = {0};
	
	AttributeUseVector attrs = {0};
	
	for(int i = 0; i < tokens.length; i++){
		Token tok = tokens.vals[i];
		
		if(TOKEN_IS(tok, "/*[")){
			i = ParseAttributes(tokens, i, &attrs);
			i--;
			
			if(!TOKEN_IS(tokens.vals[i+1], "struct") && !TOKEN_IS(tokens.vals[i+2], "struct")){
				free(attrs.vals);
				attrs.vals = NULL;
				attrs.length = 0;
				attrs.capacity = 0;
			}
			
		}
		else if(TOKEN_IS(tok, "struct")){
			StructDef structDef = {0};
			StructDefineType structDefType = INVALID;
			
			Token prevToken = {0};
			Token nextToken = {0};
			if(VectorSafeGetToken(&tokens, i-1, &prevToken)){
				if(TOKEN_IS(prevToken, "typedef")){
					if(VectorSafeGetToken(&tokens, i+1, &nextToken)){
						if(TOKEN_IS(nextToken, "{")){
							structDefType = TYPEDEF_ANON;
						}
						else{
							structDefType = TYPEDEF_NAMED;
						}
					}
					else{
						//The last token in our vector is 'struct', which can't be right;
						structDefType = INVALID;
					}
				}
				else{
					structDefType = CPP_STYLE;
				}
			}
			else{
				structDefType = CPP_STYLE;
			}
			
			if(structDefType == CPP_STYLE){
				if(!VectorSafeGetToken(&tokens, i+1, &nextToken)){
					structDefType = INVALID;
				}
			}
			
			switch(structDefType){
				
				case INVALID:{
					printf("Yo, invalid struct def over here.\n");
				}break;
				
				case TYPEDEF_NAMED:
					VectorAddToken(knownTypes, nextToken);
					//printf("Named typedef, structure name: '%.*s'\n", nextToken.length, nextToken.start);
					i += 2;
				case TYPEDEF_ANON:{
					i = ParseFields(tokens, knownTypes, i, &structDef);
					
					Token structName = {0};
					if(VectorSafeGetToken(&tokens, i+1, &structName)){
						structDef.name = structName;
						//printf("Named typedef, struct typdef is: '%.*s'\n", structName.length, structName.start);
						VectorAddToken(knownTypes, structName);
					}
					else{
						printf("Error, expected '}'");
					}
				} break;
				
				case CPP_STYLE:{
					VectorAddToken(knownTypes, nextToken);
					structDef.name = nextToken;
					//printf("   Found cpp-style struct, named '%.*s'\n", nextToken.length, nextToken.start);
					
					Token tokenAfterName = {0};
					if(VectorSafeGetToken(&tokens, i+2, &tokenAfterName)){
						if(TOKEN_IS(tokenAfterName, ":")){
							Token structParent = {0};
							if(VectorSafeGetToken(&tokens, i+3, &structParent)){
								if(TOKEN_IS(structParent, "public") || TOKEN_IS(structParent, "private") || TOKEN_IS(structParent, "protected")){
									VectorSafeGetToken(&tokens, i+4, &structParent);
								}
								
								if(FindToken(*knownTypes, structParent) != -1){
									i += 2;
									structDef.parentType = structParent;
									//printf("     Struct parent is: '%.*s'\n", structParent.length, structParent.start);
								}
								else{
									printf("     Err: struct parent is unkown type: '%.*s'\n", structParent.length, structParent.start);
								}
							}
						}
					}
					
					i +=  2;
					
					i = ParseFields(tokens, knownTypes, i, &structDef);
					
				}break;
				
			}
			
			structDef.attrs = attrs;
			attrs.length = 0;
			attrs.capacity = 0;
			attrs.vals = NULL;
			VectorAddStructDef(&structDefs, structDef);
		}
	}
	
	return structDefs;
}

int ParseEnumEntries(TokenVector tokens, int startIdx, EnumDef* def){
	int i = startIdx;
	
	AttributeUseVector attrs = {0};
	
	for( ; i < tokens.length && !TOKEN_IS(tokens.vals[i], "}"); i++){
		i = ParseAttributes(tokens, i, &attrs);
		Token tok = tokens.vals[i];
		if(TOKEN_IS(tok, "=")){
			while(i < tokens.length && !TOKEN_IS(tokens.vals[i], ",") && !TOKEN_IS(tokens.vals[i], "}")){
				i++;
			}
			i--;
		}
		else if(!TOKEN_IS(tok, ",")){
			EnumEntry entry = {0};
			entry.enumName = tok;
			entry.attrs = attrs;
			
			attrs.length = 0;
			attrs.capacity = 0;
			attrs.vals = NULL;
			
			VectorAddEnumEntry(&def->entries, entry);
		}
	}
	
	return i;
}

EnumDefVector ParseTokensForEnums(TokenVector tokens, TokenVector* knownTypes){
	AttributeUseVector attrs = {0};
	
	EnumDefVector enumDefs = {0};
	for(int i = 0; i < tokens.length; i++){
		Token tok = tokens.vals[i];
		
		//printf("Token : '%.*s'\n", tok.length, tok.start);
		if(TOKEN_IS(tok, "/*[")){
			i = ParseAttributes(tokens, i, &attrs);
			i--;
		}
		if(TOKEN_IS(tok, "enum")){
			EnumDef enumDef = {0};
			StructDefineType structDefType = INVALID;
			
			Token prevToken = {0};
			Token nextToken = {0};
			if(VectorSafeGetToken(&tokens, i-1, &prevToken)){
				if(TOKEN_IS(prevToken, "typedef")){
					if(VectorSafeGetToken(&tokens, i+1, &nextToken)){
						if(TOKEN_IS(nextToken, "{")){
							structDefType = TYPEDEF_ANON;
						}
						else{
							structDefType = TYPEDEF_NAMED;
						}
					}
					else{
						//The last token in our vector is 'struct', which can't be right;
						structDefType = INVALID;
					}
				}
				else{
					structDefType = CPP_STYLE;
				}
			}
			else{
				structDefType = CPP_STYLE;
			}
			
			if(structDefType == CPP_STYLE){
				if(!VectorSafeGetToken(&tokens, i+1, &nextToken)){
					structDefType = INVALID;
				}
			}
			
			switch(structDefType){
				
				case INVALID:{
					printf("Yo, invalid enum def over here.\n");
				}break;
				
				case TYPEDEF_NAMED:
					VectorAddToken(knownTypes, nextToken);
					i++;
				case TYPEDEF_ANON:{
					//printf("Anon enum typedef.\n");
					i += 2;
					i = ParseEnumEntries(tokens, i, &enumDef);
					
					Token enumTypeName = {0};
					if(VectorSafeGetToken(&tokens, i+1, &enumTypeName)){	
						enumDef.name = enumTypeName;
						VectorAddToken(knownTypes, enumTypeName);
					}
					else{
						printf("Error, expected '}'");
					}
				} break;
				
				case CPP_STYLE:{
					enumDef.name = nextToken;
					VectorAddToken(knownTypes, nextToken);
					i += 3;
					i = ParseEnumEntries(tokens, i, &enumDef);
				} break;
			}
			
			enumDef.attrs = attrs;
			attrs.length = 0;
			attrs.capacity = 0;
			attrs.vals = NULL;
			VectorAddEnumDef(&enumDefs, enumDef);
		}
	}
	
	return enumDefs;
}

void MetaParseFile(const char* fileName, EnumDefVector* outEnumDefs, StructDefVector* outStructDefs){
	int fileLength;
	char* fileBuffer = ReadTextFile(fileName, &fileLength);
	
	OpenFileInfo fileOp = {fileBuffer, fileLength, {fileName, _strlen(fileName)}};
	VectorAddOpenFileInfo(&openFiles, fileOp);
	
	TokenVector knownTypes = {0};
	
	Token starterTypes[] = { MAKE_TOKEN(int), MAKE_TOKEN(int8), MAKE_TOKEN(int16), 
							MAKE_TOKEN(int32), MAKE_TOKEN(uint8), MAKE_TOKEN(uint16), 
							MAKE_TOKEN(float), MAKE_TOKEN(uint32), MAKE_TOKEN(char), 
							MAKE_TOKEN(short), MAKE_TOKEN(double) };
	
	Token serialNameTok = MAKE_TOKEN(SerializeAs);
	Token gameEntTok = MAKE_TOKEN(GameEntity);
	
	for(int i = 0; i < ARRAY_COUNT(starterTypes); i++){
		VectorAddToken(&knownTypes, starterTypes[i]);
	}
	
	TokenVector tokens = PreprocessString(fileBuffer);
	
	EnumDefVector enumDefs = ParseTokensForEnums(tokens, &knownTypes);
	
	StructDefVector structDefs = ParseTokensForStructs(tokens, &knownTypes);
	
	
	*outEnumDefs = enumDefs;
	*outStructDefs = structDefs;
}