#include "lexer.c"

typedef struct{
	Token name;
	TokenVector val;
} PreprocDef;

typedef struct{
	Token name;
	TokenVector args;
	TokenVector result;
} PreprocDefFunc;

DEFINE_VECTOR(PreprocDef)
DEFINE_VECTOR(PreprocDefFunc)

DEFINE_VECTOR(TokenVector)

typedef struct{
	PreprocDefVector simpleDefs;
	PreprocDefFuncVector funcDefs;
	
	int ifLevels;
} PrepocContext;

TokenVector PreprocessString(char* string);

typedef struct{
	char* start;
	int length;
	Token fileName;
} OpenFileInfo;

DEFINE_VECTOR(OpenFileInfo)

OpenFileInfoVector openFiles = {0};

TokenVector GetAndRemoveDirectiveTokens(TokenVector* tokens, int index){
	Token startTok = tokens->vals[index];
	const char* cursor = startTok.start;
	
	while(*cursor && *cursor != '\n' && *cursor != '\r'){
		cursor++;
	}
	cursor--;
	
	//printf("Getting directive: '%.*s'\n", cursor - startTok.start, startTok.start);
	
	TokenVector directiveToks = {0};
	
	int tokCount = 0;
	for(int i = index; i < tokens->length; i++){
		if(tokens->vals[i].start > cursor || tokens->vals[i].start < startTok.start){
			break;
		}
		tokCount++;
	}
	
	for(int i = 0; i < tokCount; i++){
		VectorAddToken(&directiveToks, tokens->vals[index+i]);
	}
	
	for(int i = 0; i < tokCount; i++){	
		VectorRemoveToken(tokens, index);
	}
	
	return directiveToks;
}

void ParseDirective(TokenVector tokens, TokenVector* fileTokens, int index, PrepocContext* context){
	if(tokens.length < 2 || !TOKEN_IS(tokens.vals[0], "#")){
		printf("\n\nError: malformed preproc directive (length %d): \n", tokens.length);
		
		for(int i = 0; i < tokens.length; i++){
			printf("'%.*s', ", tokens.vals[i].length, tokens.vals[i].start);
		}
		
		printf("\n");
	}
	
	Token directiveType = tokens.vals[1];
	//printf("Directive type: '%.*s'\n", tokens.vals[1].length, tokens.vals[1].start);
	
	if(context->ifLevels > 0){
		if(TOKEN_IS(directiveType, "if")){
			context->ifLevels++;
		}
		else if(TOKEN_IS(directiveType, "ifdef")){
			context->ifLevels++;
		}
		else if(TOKEN_IS(directiveType, "ifndef")){
			context->ifLevels++;
		}
		else if(TOKEN_IS(directiveType, "endif")){
			context->ifLevels--;
		}
	}
	else{
		if(TOKEN_IS(directiveType, "include")){
			Token data = tokens.vals[2];
			if(TOKEN_IS(data, "<")){
				//TODO: Fix up issues
			}
			else{
				
				int fileOpenIdx = -1;
				for(int i = 0; i < openFiles.length; i++){
					if(data.start > openFiles.vals[i].start && data.start < (openFiles.vals[i].start + openFiles.vals[i].length)){
						fileOpenIdx = i;
						break;
					}
				}
				
				Token currentFileName = openFiles.vals[fileOpenIdx].fileName;
				while(currentFileName.length > 0 && currentFileName.start[currentFileName.length] != '/'){
					currentFileName.length--;
				}
				
				char path[256] = {0};
				snprintf(path, 256, "%.*s/%.*s", currentFileName.length, currentFileName.start, data.length - 2, data.start + 1);
				
				int fileSize;
				char* fileBuffer = ReadTextFile(path, &fileSize);
				
				OpenFileInfo fileOp = {fileBuffer, fileSize, {data.start+1, data.length-2}};
				VectorAddOpenFileInfo(&openFiles, fileOp);
				
				printf("About to process included file.\n");
				TokenVector lexedIncludedFile = PreprocessString(fileBuffer);
				printf("Processed included file.\n");
				VectorInsertVectorToken(fileTokens, index, lexedIncludedFile);
				printf("Added included tokens.\n");
			}
		}
		else if(TOKEN_IS(directiveType, "define")){
			Token macroName = tokens.vals[2];
			
			if(tokens.length > 3 && TOKEN_IS(tokens.vals[3], "(") && tokens.vals[2].start + tokens.vals[2].length == tokens.vals[3].start){
				//printf("Preproc func\n");
				
				PreprocDefFunc defFunc = {0};
				
				defFunc.name = macroName;
				
				int i = 4;
				for( ; i < tokens.length; i++){
					if(TOKEN_IS(tokens.vals[i], ")")){
						i++;
						break;
					}
					else if(!TOKEN_IS(tokens.vals[i], ",")){
						VectorAddToken(&defFunc.args, tokens.vals[i]);
					}
				}
				
				for( ; i < tokens.length; i++){
					VectorAddToken(&defFunc.result, tokens.vals[i]);
				}
				
				VectorAddPreprocDefFunc(&context->funcDefs, defFunc);
			}
			else{
				//printf("Preproc subst def.\n");
				
				PreprocDef def = {0};
				
				def.name = macroName;
				for(int i = 3; i < tokens.length; i++){
					VectorAddToken(&def.val, tokens.vals[i]);
				}
				
				//printf("Preproc define '%.*s' && '%.*s'\n", def.name.length, def.name.start, def.val.vals[0].length, def.val.vals[0].start);
				
				VectorAddPreprocDef(&context->simpleDefs, def);
			}
		}
		else if(TOKEN_IS(directiveType, "if")){
			
		}
		else if(TOKEN_IS(directiveType, "ifdef")){
			int isDefined = 0;
			for(int i = 0; i < context->simpleDefs.length; i++){
				if(TokenEqual(context->simpleDefs.vals[i].name, tokens.vals[2])){
					isDefined = 1;
					break;
				}
			}
			for(int i = 0; i < context->funcDefs.length; i++){
				if(TokenEqual(context->funcDefs.vals[i].name, tokens.vals[2])){
					isDefined = 1;
					break;
				}
			}
			
			if(!isDefined){
				printf("Found ifdef that is not defined.\n");
				context->ifLevels = 1;
			}
		}
		else if(TOKEN_IS(directiveType, "ifndef")){
			int isDefined = 0;
			for(int i = 0; i < context->simpleDefs.length; i++){
				if(TokenEqual(context->simpleDefs.vals[i].name, tokens.vals[2])){
					isDefined = 1;
					break;
				}
			}
			for(int i = 0; i < context->funcDefs.length; i++){
				if(TokenEqual(context->funcDefs.vals[i].name, tokens.vals[2])){
					isDefined = 1;
					break;
				}
			}
			
			if(isDefined){
				context->ifLevels = 1;
			}
		}
		else if(TOKEN_IS(directiveType, "line")){
			//TODO
		}
		else if(TOKEN_IS(directiveType, "undef")){
			for(int i = 0; i < context->simpleDefs.length; i++){
				if(TokenEqual(context->simpleDefs.vals[i].name, tokens.vals[2])){
					VectorRemovePreprocDef(&context->simpleDefs, i);
					break;
				}
			}
			for(int i = 0; i < context->funcDefs.length; i++){
				if(TokenEqual(context->funcDefs.vals[i].name, tokens.vals[2])){
					VectorRemovePreprocDefFunc(&context->funcDefs, i);
					break;
				}
			}
		}
		else if(TOKEN_IS(directiveType, "endif")){
			//Do nothing, we're closing off an active if block
		}
	}
}

TokenVector PreprocessString(char* string){
	
	PrepocContext context = {0};
	
	TokenVector lexedFile = LexString(string);
	
	for(int i = 0; i < lexedFile.length; i++){
		Token tok = lexedFile.vals[i];
		//printf("Tok: '%.*s'\n", tok.length, tok.start);
		if(TOKEN_IS(tok, "#")){
			TokenVector directiveToks = GetAndRemoveDirectiveTokens(&lexedFile, i);
			//printf("Directive toks (length %d):\n", directiveToks.length);
			ParseDirective(directiveToks, &lexedFile, i, &context);
			i--;
		}
		else if(context.ifLevels > 0){
			//printf("Removing token: '%.*s'\n", tok.length, tok.start);
			VectorRemoveToken(&lexedFile, i);
			i--;
		}
		else{
			int expandDef = 0;
			for(int j = 0; j < context.simpleDefs.length; j++){
				if(TokenEqual(tok, context.simpleDefs.vals[j].name)){
					VectorRemoveToken(&lexedFile, i);
					VectorInsertVectorToken(&lexedFile, i, context.simpleDefs.vals[j].val);
					expandDef = 1;
					break;
				}
			}
			for(int j = 0; j < context.funcDefs.length; j++){
				if(TokenEqual(tok, context.funcDefs.vals[j].name)){
					//printf("FOUND ARG FUNC.\n");
					int endIdx = i + 1;
					while(!TOKEN_IS(lexedFile.vals[endIdx], ")")){
						endIdx++;
					}
					
					TokenVectorVector argList = {0};
					
					TokenVector emptyVec = {0};
					VectorAddTokenVector(&argList, emptyVec);
					
					int argIdx = 0;
					for(int k = i + 2; k < endIdx; k++){
						if(TOKEN_IS(lexedFile.vals[k], ",")){
							argIdx++;
							VectorAddTokenVector(&argList, emptyVec);
						}
						else{
							VectorAddToken(&argList.vals[argIdx], lexedFile.vals[k]);
						}
					}
					
					for(int k = i; k <= endIdx; k++){
						VectorRemoveToken(&lexedFile, i);
					}
					
					
					for(int funcIdx = context.funcDefs.vals[j].result.length - 1; funcIdx >= 0; funcIdx--){
						int isArg = 0;
						for(int argIdx = 0; argIdx < context.funcDefs.vals[j].args.length; argIdx++){
							if(TokenEqual(context.funcDefs.vals[j].args.vals[argIdx], context.funcDefs.vals[j].result.vals[funcIdx])){
								VectorInsertVectorToken(&lexedFile, i, argList.vals[argIdx]);
								isArg = 1;
								break;
							}
						}

						if (!isArg) {
							//printf("lexedFile.length ~: %d\n", lexedFile.length);
							VectorInsertToken(&lexedFile, i, context.funcDefs.vals[j].result.vals[funcIdx]);
						}
					}
					
					expandDef = 1;
					break;
				}
			}
			
			if(expandDef){
				i--;
			}
		}
	}
	
	return lexedFile;
}

/*
int main(int argc, char** argv){
	int fileSize;
	const char* fileBuffer = ReadTextFile("test2.h", &fileSize);
	
	
	}
	
	printf("Here's the preprocessed test:\n");
	for(int i = 0; i < lexedFile.length; i++){
		printf("%.*s ", lexedFile.vals[i].length, lexedFile.vals[i].start);
		if(TOKEN_IS(lexedFile.vals[i], ";") || TOKEN_IS(lexedFile.vals[i], "{") || TOKEN_IS(lexedFile.vals[i], "}")){
			printf("\n");
		}
	}
	//printf("%s\n", outBuffer.start);
	
	return 0;
}
*/