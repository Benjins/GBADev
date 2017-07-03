#include <stdio.h>

typedef unsigned char uint8;

#define bool int
#define true 1
#define false 0

typedef struct{
	char* str;
	int len;
} ParseToken;

#define VEC_PARAM ParseToken
#include "vec.h"

#define BNS_ARRAY_COUNT(x) (sizeof(x) / sizeof((x)[0]))
#define BNS_ABS(x) ((x) >= 0 ? (x) : -(x))

const char* punctuation[] = {
	"@", ",", ".", "{", "}", "[", "]", ":", "#", "<<", ">>"
};

typedef enum{
	LS_Whitespace,
	LS_Comment,
	LS_Identifier,
	LS_Punctuation
} LexerState;

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

static inline bool IsNumber(const char* c){
	return (*c >= '0' && *c <= '9') || (*c == '-' && c[1] >= '0' && c[1] <= '9');
}

int ParseInt(const char* str, int len) {
	const char* cursor = str;
	int val = 0;
	int sign = 1;
	bool isHex = false;

	if (*cursor == '-') {
		cursor++;
		sign = -1;
	}
	else if (*cursor == '0' && cursor[1] == 'x') {
		cursor += 2;
		isHex = true;
	}

	const int base = (isHex ? 16 : 10);
	while (*cursor && (cursor - str) < len) {
		int digit = (*cursor - '0');
		
		if (isHex && *cursor >= 'A' && *cursor <= 'F') {
			digit = (*cursor - 'A') + 10;
		}

		val *= base;
		val += digit;
		cursor++;
	}

	return val * sign;
}

static inline bool TokenEq(ParseToken a, ParseToken b){
	return (a.len == b.len) && _memeq(a.str, b.str, a.len);
}

static inline bool TokenEqStr(ParseToken a, const char* b){
	return _memeq(a.str, b, a.len) && b[a.len] == '\0';
}

static inline int FindTokenInStringArray(ParseToken tok, const char** arr, int arrLen){
	for (int i = 0; i < arrLen; i++){
		if (TokenEqStr(tok, arr[i])){
			return i;
		}
	}
	
	return -1;
}

char* ReadStringFromFile(const char* fileName, int* outLen){
	FILE* fIn = fopen(fileName, "rb");
	if (fIn == NULL){
		if (outLen != NULL){
			*outLen = 0;
		}
		
		return NULL;
	}
	
	fseek(fIn, 0, SEEK_END);
	int fileLen = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);
	
	char* fileBuffer = (char*)malloc(fileLen + 1);
	fread(fileBuffer, 1, fileLen, fIn);
	
	fclose(fIn);
	
	fileBuffer[fileLen] = '\0';
	
	if (outLen != NULL){
		*outLen = fileLen;
	}
	
	return fileBuffer;
}

ParseTokenVector LexTokensFromString(char* str, int len){
	ParseTokenVector toks = {0};
	
	LexerState state = LS_Whitespace;
	char* cursor = str;
	ParseToken tok = {0};
	tok.str = cursor;
	
	#define EMIT_TOKEN() \
	do { \
		tok.len = cursor - tok.str; \
		if (tok.len > 0){ \
			ParseTokenVectorPushBack(&toks, tok); \
		} \
	} while(0)
		
	#define START_TOKEN() tok.str = cursor; tok.len = 1;
	
	while(*cursor && (cursor - str) < len){
		bool isWhitespace = *cursor == '\n' || *cursor == '\r' || *cursor == '\t' || *cursor == ' ';
		bool isComment = *cursor == ';';
		
		if (isComment){
			if (state != LS_Whitespace && state != LS_Comment	){
				EMIT_TOKEN();
			}
			state = LS_Comment;
		}
		
		switch (state){
			case LS_Whitespace: {
				if (!isWhitespace){
					START_TOKEN();
					
					bool isPunc = false;
					for (int i = 0; i < BNS_ARRAY_COUNT(punctuation); i++){
						if (punctuation[i][0] == *cursor){
							isPunc = true;
							break;
						}
					}
					
					state = isPunc ? LS_Punctuation : LS_Identifier;
				}
			} break;
			case LS_Comment: {
				if (*cursor == '\n'){
					state = LS_Whitespace;
				}
			} break;
			case LS_Identifier: {
				if (isWhitespace){
					EMIT_TOKEN();
					state = LS_Whitespace;
				}
				else{
					bool isPunc = false;
					for (int i = 0; i < BNS_ARRAY_COUNT(punctuation); i++){
						if (punctuation[i][0] == *cursor){
							isPunc = true;
							break;
						}
					}
					
					if (isPunc){
						EMIT_TOKEN();
						START_TOKEN();
						state = LS_Punctuation;
					}
				}
			} break;
			case LS_Punctuation: {
				bool isPunc = false;
				for (int i = 0; i < BNS_ARRAY_COUNT(punctuation); i++){
					if (TokenEqStr(tok, punctuation[i])){
						isPunc = true;
						break;
					}
				}
				
				if (!isPunc){
					EMIT_TOKEN();
					cursor--;
					state = LS_Whitespace;
				}
			} break;
		}
		
		cursor++;
		tok.len++;
	}
	
	EMIT_TOKEN();

#undef EMIT_TOKEN
#undef START_TOKEN

	return toks;
}

const char* regNames[16][2] = {
	{"r0", ""},
	{"r1", ""},
	{"r2", ""},
	{"r3", ""},
	{"r4", ""},
	{"r5", ""},
	{"r6", ""},
	{"r7", ""},
	{"r8", ""},
	{"r9", ""},
	{"r10", "sl"},
	{"r11", "fp"},
	{"r12", "ip"},
	{"r13", "sp"},
	{"r14", "lr"},
	{"r15", "pc"}
};

int GetRegNum(ParseToken tok){
	for (int i = 0; i < 16; i++){
		for (int j = 0; j < 2; j++){
			if (TokenEqStr(tok, regNames[i][j])){
				return i;
			}
		}
	}
	
	return -1;
}

#define COND_CODE_DATA(mac) \
	mac("eq", EQ) \
    mac("ne", NE) \
    mac("cs", CS) \
    mac("cc", CC) \
    mac("mi", MI) \
    mac("pl", PL) \
    mac("vs", VS) \
    mac("vc", VC) \
    mac("hi", HI) \
    mac("ls", LS) \
    mac("ge", GE) \
    mac("lt", LT) \
    mac("gt", GT) \
    mac("le", LE) \
    mac("al", AL) \
	mac("xx", XX)
	
#define MAC(name, type) name ,
const char* condCodeNames[] = {
	COND_CODE_DATA(MAC)
};
#undef MAC

#define MAC(name, type) CCT_ ## type ,
typedef enum{
	COND_CODE_DATA(MAC)
} CondCodeType;
#undef MAC

typedef enum{
	VT_None = 0,
	VT_ShiftedRegister = (1 << 0),
	VT_Register        = (1 << 1),
	VT_Immediate       = (1 << 2),
	VT_RegisterList    = (1 << 3),
	VT_Label           = (1 << 4),
	VT_RegisterOffset =  (1 << 5),
	VT_All = (VT_RegisterOffset << 1) - 1,
	VT_2ndOperand = VT_Register | VT_ShiftedRegister | VT_Immediate
} ValueType;

typedef struct{
	ValueType type;
	union{
		int registerIndex;
		
		struct{
			unsigned int shiftedRegisterIndex : 4;
			
			// negative values are shift right, positive are shift left
			int shiftedRegisterShift : 16;
		};
		
		int immediateValue;
		
		unsigned int registerList;
		
		struct{
			unsigned int offsetRegisterIndex : 4;
			int offsetRegisterOffset : 16;
		};
		
		struct{
			union{
				ParseToken labelName;
				unsigned int labelAddr;
			};
			
			bool labelResolved;
		};
	};
} AsmValue;

bool IsLabel(ParseTokenVector labels, ParseToken tok){
	for (int i = 0; i < labels.count; i++){
		if (TokenEq(labels.data[i], tok)){
			return true;
		}
	}
	
	return false;
}

int ParseAsmValue(ParseTokenVector toks, int index, ValueType types, AsmValue* valOut){
	int type = 1;
	while (type & VT_All){
		if (type & types){
			// Attempt to parse the value
			switch(type){
				case VT_ShiftedRegister: {
					int regIndex = GetRegNum(toks.data[index]);
					if (regIndex >= 0){
						if (TokenEqStr(toks.data[index + 1], "<<")){
							int shift = ParseInt(toks.data[index + 2].str, toks.data[index + 2].len);
							valOut->type = (ValueType)type;
							valOut->shiftedRegisterIndex = regIndex;
							valOut->shiftedRegisterShift = shift;
							
							return index + 3;
						}
						else if (TokenEqStr(toks.data[index + 1], ">>")){
							int shift = ParseInt(toks.data[index + 2].str, toks.data[index + 2].len);
							valOut->type = (ValueType)type;
							valOut->shiftedRegisterIndex = regIndex;
							valOut->shiftedRegisterShift = -shift;
							
							return index + 3;
						}
					}
				} break;
				
				case VT_Register: {
					int regIndex = GetRegNum(toks.data[index]);
					if (regIndex >= 0){
						valOut->type = (ValueType)type;
						valOut->registerIndex = regIndex;
						return index + 1;
					}
				} break;
				
				case VT_Immediate: {
					if (IsNumber(toks.data[index].str)){
						int immVal = ParseInt(toks.data[index].str, toks.data[index].len);
						valOut->type = (ValueType)type;
						valOut->immediateValue = immVal;
						return index + 1;
					}
				} break;
				
				case VT_RegisterList: {
					if (TokenEqStr(toks.data[index], "{")){
						int newIndex = index + 1;
						bool isCorrect = true;
						unsigned int regBitfield = 0;
						
						while (!TokenEqStr(toks.data[newIndex], "}")){
							int regNum = GetRegNum(toks.data[newIndex]);
							if (regNum >= 0){
								regBitfield |= (1 << regNum);
							}
							else{
								isCorrect = false;
								break;
							}
						}
						
						if (isCorrect){
							valOut->type = (ValueType)type;
								valOut->registerList = regBitfield;
							return newIndex + 1;
						}
					}
				} break;
				
				case VT_Label: {
					if (TokenEqStr(toks.data[index], ":")){
						valOut->type = (ValueType)type;
						valOut->labelName = toks.data[index + 1];

						return index + 2;
					}
				} break;
				
				case VT_RegisterOffset: {
					if (TokenEqStr(toks.data[index], "[")){
						int regNum = GetRegNum(toks.data[index + 1]);
						if (regNum >= 0){
							if (TokenEqStr(toks.data[index + 2], "]")){
								valOut->type = (ValueType)type;
								valOut->offsetRegisterIndex = regNum;
								valOut->offsetRegisterOffset = 0;

								return index + 3;
							}
							else if(IsNumber(toks.data[index + 2].str) 
								&& TokenEqStr(toks.data[index + 3], "]")){
								valOut->type = (ValueType)type;
								valOut->offsetRegisterIndex = regNum;
								valOut->offsetRegisterOffset = ParseInt(toks.data[index + 2].str, toks.data[index + 2].len);
								
								return index + 4;
							}
						}
					}
				} break;
				
				default: {
					printf("This shouldn't have happened...\n");
					return index + 1;
				} break;
			}
		}
		
		type <<= 1;
	}
	
	return index;
}

#define OP_CODE_DATA(mac) \
	mac("and",  AND ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("eor",  EOR ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("sub",  SUB ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("rsb",  RSB ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("add",  ADD ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("adc",  ADC ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("sbc",  SBC ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("rsc",  RSC ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("tst",  TST ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("teq",  TEQ ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("cmp",  CMP ,  VT_Register,  VT_2ndOperand,                VT_None) \
	mac("cmn",  CMN ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("orr",  ORR ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("mov",  MOV ,  VT_Register,  VT_2ndOperand | VT_Label,     VT_None) \
	mac("bic",  BIC ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("mvn",  MVN ,  VT_Register,  VT_2ndOperand | VT_Label,     VT_None) \
	mac("lsl",  LSL ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("lsr",  LSR ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("asl",  ASL ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("asr",  ASR ,  VT_Register,  VT_Register,                  VT_2ndOperand) \
	mac("ldr",  LDR ,  VT_Register,  VT_RegisterOffset | VT_Label, VT_None) \
	mac("str",  STR ,  VT_Register,  VT_RegisterOffset | VT_Label, VT_None) \
	mac("ldrh", LDRH,  VT_Register,  VT_RegisterOffset | VT_Label, VT_None) \
	mac("strh", STRH,  VT_Register,  VT_RegisterOffset | VT_Label, VT_None) \
	mac("b",    B   ,  VT_Label ,    VT_None,                      VT_None) \
	mac("bx",   BX  ,  VT_Register,  VT_None,                      VT_None) \
	mac("bl",   BL  ,  VT_Label,     VT_None,                      VT_None) \
	mac("swi",  SWI ,  VT_Immediate, VT_None,                      VT_None)


#define MAC(str, name, op1Type, op2Type, op3Type) str ,
const char* opCodeNames[] = {
	OP_CODE_DATA(MAC)
	"LIT"
};
#undef MAC

typedef struct{
	unsigned int op1Type : 10;
	unsigned int op2Type : 10;
	unsigned int op3Type : 10;
} InstructionType;

#define MAC(str, name, op1Type, op2Type, op3Type) { op1Type , op2Type , op3Type },
InstructionType opCodeTypes[] = {
	OP_CODE_DATA(MAC)
};
#undef MAC

#define MAC(str, name, op1Type, op2Type, op3Type) OC_ ## name ,
typedef enum{
	OP_CODE_DATA(MAC)
	OC_LiteralWord
} OpCode;
#undef MAC

typedef struct{
	ValueType arg1;
	ValueType arg2;
	ValueType arg3;
} OpCodeValueType;

#define MAC(str, name, op1Type, op2Type, op3Type) { op1Type , op2Type } ,
OpCodeValueType opCodeValueTypes[] = {
	OP_CODE_DATA(MAC)
};
#undef MAC

void PrintRegisterList(FILE* fp, int regList){
	fprintf(fp, "{");
	
	bool firstReg = true;
	for (int i = 0; i < 16; i++){
		if (regList & (1 << i)){
			fprintf(fp, "%sr%d", (firstReg ? "" : ", "), i);
			firstReg = false;
		}
	}
	fprintf(fp, "}");
}

#include "arm_inst.h"

static_assert(OC_MVN == 15);

typedef struct {
	OpCode opCode;
	CondCodeType cond;
	AsmValue arg1;
	AsmValue arg2;
	AsmValue arg3;
} AsmOp;

#define VEC_PARAM AsmOp
#include "vec.h"

typedef struct {
	ParseToken name;
	int ipOffset;
} LocalLabel;

#define VEC_PARAM LocalLabel
#include "vec.h"

typedef struct {
	ParseToken procName;
	AsmOpVector ops;
	LocalLabelVector localLabels;
} ProcDef;

#define VEC_PARAM ProcDef
#include "vec.h"

ProcDefVector ParseTokens(ParseTokenVector toks, LocalLabelVector* outGlobalLabels){
	ProcDefVector procDefs = { 0 };
	ProcDef currentDef = { 0 };
	LocalLabelVector globalLabels = { 0 };

	for (int i = 0; i < toks.count; i++){
		if (TokenEqStr(toks.data[i], "@")){
			i++;
			if (TokenEqStr(toks.data[i], "include")) {
				// TODO
			}
			else if (TokenEqStr(toks.data[i], "label")) {
				i++;
				LocalLabel label = { 0 };
				label.name = toks.data[i];

				int offset = currentDef.ops.count;
				for (int i = 0; i < procDefs.count; i++) {
					offset += procDefs.data[i].ops.count;
				}
				label.ipOffset = offset;

				LocalLabelVectorPushBack(&globalLabels, label);
			}
			else if (TokenEqStr(toks.data[i], "labelWord")) {
				i++;
				AsmOp op = { 0 };
				op.opCode = OC_LiteralWord;
				op.arg1.type = VT_Label;
				op.arg1.labelName = toks.data[i];
				AsmOpVectorPushBack(&currentDef.ops, op);
			}
			else if (TokenEqStr(toks.data[i], "word")) {
				i++;

				AsmOp op = { 0 };
				op.opCode = OC_LiteralWord;
				op.arg1.type = VT_Immediate;
				op.arg1.immediateValue = ParseInt(toks.data[i].str, toks.data[i].len);

				AsmOpVectorPushBack(&currentDef.ops, op);
			}
			else if (TokenEqStr(toks.data[i], "fill")) {
				i++;

				int count = ParseInt(toks.data[i].str, toks.data[i].len);
				i++;

				if (count < 0) {
					printf("\nError: improper count in @fill: %d\n", count);
					count = 0;
				}

				for (int i = 0; i < count; i++) {
					AsmOp op = { 0 };
					op.opCode = OC_LiteralWord;
					op.arg1.type = VT_Immediate;
					op.arg1.immediateValue = ParseInt(toks.data[i].str, toks.data[i].len);
					AsmOpVectorPushBack(&currentDef.ops, op);
				}
			}
			else if (TokenEqStr(toks.data[i], "proc")) {
				i++;
				ParseToken procName = toks.data[i];

				currentDef.procName = procName;

				LocalLabel label = { 0 };
				label.name = procName;

				int offset = 0;
				for (int i = 0; i < procDefs.count; i++) {
					offset += procDefs.data[i].ops.count;
				}
				label.ipOffset = offset;

				LocalLabelVectorPushBack(&globalLabels, label);
			}
			else if (TokenEqStr(toks.data[i], "endproc")) {
				ProcDefVectorPushBack(&procDefs, currentDef);

				// Better way to reset? :p
				ProcDef nextDef = { 0 };
				currentDef = nextDef;
			}
			else {
				printf("\nError, unknown directive @%.*s\n", toks.data[i].len, toks.data[i].str);
			}
		}
		else {
			if (TokenEqStr(toks.data[i], ":")) {
				i++;
				ParseToken labelName = toks.data[i];
				i++;
				if (TokenEqStr(toks.data[i], ":")) {
					//printf("Label '%.*s':\n", labelName.len, labelName.str);
					LocalLabel label;
					label.name = labelName;
					label.ipOffset = currentDef.ops.count;

					LocalLabelVectorPushBack(&currentDef.localLabels, label);
				}
				else {
					// TODO: parse error
				}
			}
			else {
				int opCodeIdx = FindTokenInStringArray(toks.data[i], opCodeNames, BNS_ARRAY_COUNT(opCodeNames));

				if (opCodeIdx < 0) {
					printf("Shrug...'%.*s'\n", toks.data[i].len, toks.data[i].str);
				}
				else {
					CondCodeType condCode = CCT_AL;
					i++;
					if (TokenEqStr(toks.data[i], ".")) {
						i++;

						int condCodeIdx = FindTokenInStringArray(toks.data[i], condCodeNames, BNS_ARRAY_COUNT(condCodeNames));
						if (condCodeIdx >= 0) {
							condCode = (CondCodeType)condCodeIdx;
						}
						else {
							printf("Error, incorrect cond code: %.*s", toks.data[i].len, toks.data[i].str);
						}
						i++;
					}

					AsmValue firstOp = { 0 }, secondOp = { 0 }, thirdOp = { 0 };
					int newIdx = ParseAsmValue(toks, i, opCodeTypes[opCodeIdx].op1Type, &firstOp);
					newIdx = ParseAsmValue(toks, newIdx, opCodeTypes[opCodeIdx].op2Type, &secondOp);
					newIdx = ParseAsmValue(toks, newIdx, opCodeTypes[opCodeIdx].op3Type, &thirdOp);

					bool missingSecondOp = (secondOp.type == VT_None && opCodeTypes[opCodeIdx].op2Type != VT_None);
					bool missingThirdOp = (thirdOp.type == VT_None && opCodeTypes[opCodeIdx].op3Type != VT_None);
					if (firstOp.type == VT_None || missingSecondOp || missingThirdOp) {
						printf("Unable to parse op code '%.*s'\n", toks.data[i].len, toks.data[i].str);
						break;
					}
					else {
						AsmOp op = {0};
						op.cond = condCode;
						op.opCode = opCodeIdx;
						op.arg1 = firstOp;
						op.arg2 = secondOp;
						op.arg3 = thirdOp;
						AsmOpVectorPushBack(&currentDef.ops, op);
						i = newIdx - 1;
					}
				}
			}
		}
	}

	*outGlobalLabels = globalLabels;

	return procDefs;
}

static inline bool UnpackImmediate(unsigned int val, int* outByte, int* outShift) {
	if (val == 0) {
		*outByte = 0;
		*outShift = 0;
		return true;
	}

	for (int i = 0; i < 15; i++) {
		if ((val & 3) &&  val < 256) {
			*outByte = val;
			*outShift = i;
			return true;
		}
		else if (val & 3) {
			return false;
		}
		val >>= 2;
	}

	return false;
}

bool SecondOperand(ArmInstruction* inst, AsmValue op) {
	if (inst->imm) {
		int immByte = 0, immShift = 0;
		bool success = UnpackImmediate(op.immediateValue, &immByte, &immShift);
		if (!success) {
			printf("Error, constant %d too big for immediate value.\n", op.immediateValue);
			return false;
		}

		inst->immByte = immByte;
		inst->immShift = (immShift ? 16 - immShift : 0);
	}
	else if (op.type == VT_Register) {
		inst->op2 = op.registerIndex;
	}
	else {
		inst->shiftedDstReg = op.shiftedRegisterIndex;
		inst->_shiftedDstReserved = (op.shiftedRegisterShift >= 0 ? 0 : 2);
		inst->shiftedDstShift = BNS_ABS(op.shiftedRegisterShift);
	}

	return true;
}

ArmInstruction AsmOpToMachineInstruction(AsmOp op, int opAddr) {
	ArmInstruction inst = { 0 };
	inst.condCode = op.cond;
	if (op.opCode >= OC_AND && op.opCode <= OC_MVN) {
		inst._instType = 0;
		inst.opCode = op.opCode;
		if (op.opCode == OC_CMP) {
			inst.imm = (op.arg2.type == VT_Immediate);
			inst.regOp1 = op.arg1.registerIndex;
			inst.setCondFlags = 1;
		}
		else if (op.opCode == OC_MOV
			  || op.opCode == OC_MVN) {
			inst.imm = (op.arg2.type == VT_Immediate);
			inst.regOp1 = 0;
			inst.regDst = op.arg1.registerIndex;

			SecondOperand(&inst, op.arg2);
		}
		else {
			inst.imm = (op.arg3.type == VT_Immediate);
			inst.setCondFlags = 0;

			inst.regDst = op.arg1.registerIndex;
			inst.regOp1 = op.arg2.registerIndex;

			SecondOperand(&inst, op.arg3);
		}
	}
	else if (op.opCode >= OC_B && op.opCode <= OC_BL) {
		if (op.opCode == OC_B) {
			inst.branchXL = 10;
			inst.branchOffset = (op.arg1.labelAddr - opAddr) - 2;
		}
		else if (op.opCode == OC_BL) {
			inst.branchXL = 11;
			inst.branchOffset = (op.arg1.labelAddr - opAddr) - 2;
		}
		else {
			inst.branchXL = 1;
			inst.branchOffset = (0x2FFF1 << 4);
			inst.regM = op.arg1.registerIndex;
		}
	}
	else if (op.opCode == OC_LDR || op.opCode == OC_STR) {
		inst._instType = 1;
		inst.regDst = op.arg1.registerIndex;
		inst.ldrStrIsLoad = (op.opCode == OC_LDR);

		inst.ldrStrPreOffset = 1;
		if (op.arg2.type == VT_RegisterOffset) {
			int offset = op.arg2.offsetRegisterOffset;
			inst.ldrStrOffsetSign = (offset >= 0);
			inst.ldrStrOffset = BNS_ABS(offset);
			inst.regOp1 = op.arg2.offsetRegisterIndex;
		}
		else {
			inst.ldrStrOffset = 0;
			inst.regOp1 = op.arg2.registerIndex;
		}
	}
	else if (op.opCode == OC_LDRH || op.opCode == OC_STRH) {
		
		inst.ldrStrHalfWordReserved1 = 1;
		inst.ldrStrHalfWordPreOffset = 1;
		inst.ldrStrHalfWordReserved2 = 0;
		inst.ldrStrHalfWordReserved = 0x0B;
		inst.ldrStrHalfWordDestReg = op.arg1.registerIndex;

		inst.ldrStrHalfWordIsLoad = (op.opCode == OC_LDRH);

		if (op.arg2.type == VT_RegisterOffset) {
			int offset = op.arg2.offsetRegisterOffset;
			inst.ldrStrHalfWordOffsetSign = (offset >= 0);
			offset = BNS_ABS(offset);
			inst.ldrStrHalfWordOffsetLowNibble = offset & 0xF;
			inst.ldrStrHalfWordOffsetHighNibble = (offset >> 4) & 0xF;
			inst.ldrStrHalfWordBaseReg = op.arg2.offsetRegisterIndex;
		}
		else {
			inst.ldrStrHalfWordBaseReg = op.arg2.registerIndex;
			inst.ldrStrHalfWordOffsetLowNibble = 0;
			inst.ldrStrHalfWordOffsetHighNibble = 0;
		}
	}
	else if (op.opCode >= OC_LSL && op.opCode <= OC_ASR) {
		// TODO
	}
	else if (op.opCode == OC_SWI){
		inst.swiImmediate = op.arg1.immediateValue << 16;
		inst._swiReserved = 0x0F;
	}
	else if (op.opCode == OC_LiteralWord) {
		if (op.arg1.type == VT_Immediate) {
			inst.val = op.arg1.immediateValue;
		}
		else if (op.arg1.type == VT_Label) {
			inst.val = op.arg1.labelAddr * 4;
		}
		else {
			// TODO: Error
		}
	}

	return inst;
}

int GetRawLabelAddr(ParseToken labelName, LocalLabelVector labels) {
	for (int i = 0; i < labels.count; i++) {
		if (TokenEq(labels.data[i].name, labelName)) {
			return labels.data[i].ipOffset;
		}
	}

	return -1;
}

int GetOffsetOfLabel(ProcDef* def, ParseToken labelName, LocalLabelVector globalLabels) {
	int localOffset = GetRawLabelAddr(labelName, def->localLabels);
	if (localOffset >= 0) {
		return localOffset;
	}

	for (int i = 0; i < globalLabels.count; i++) {
		if (TokenEq(globalLabels.data[i].name, labelName)) {
			int jumpToAddr = globalLabels.data[i].ipOffset;
			int jumpFromAddr = 0;
			for (int j = 0; j < globalLabels.count; j++) {
				if (TokenEq(globalLabels.data[j].name, def->procName)) {
					jumpFromAddr = globalLabels.data[j].ipOffset;
					break;
				}
			}

			return jumpToAddr - jumpFromAddr;
		}
	}

	return -1;
}

void LinkLocalLabels(ProcDef* def, LocalLabelVector globalLabels) {
	for (int i = 0; i < def->ops.count; i++) {
		if (def->ops.data[i].arg1.type == VT_Label) {
			if (def->ops.data[i].opCode == OC_LiteralWord) {
				def->ops.data[i].arg1.labelAddr = GetRawLabelAddr(def->ops.data[i].arg1.labelName, globalLabels);
			}
			else {
				def->ops.data[i].arg1.labelAddr = GetOffsetOfLabel(def, def->ops.data[i].arg1.labelName, globalLabels);
			}
		}
		if (def->ops.data[i].arg2.type == VT_Label) {
			def->ops.data[i].arg2.labelAddr = GetOffsetOfLabel(def, def->ops.data[i].arg2.labelName, globalLabels);
		}

		// I don't think arg3 can have labels...
	}
}

static inline unsigned int SwitchEndianness(unsigned int x) {
	return ((x & 0xFF) << 24) | ((x & 0xFF00) << 8) | ((x & 0xFF0000) >> 8) | ((x & 0xFF000000) >> 24);
}

int main(int argc, char** argv){
	int len;
	char* fileContents = ReadStringFromFile("test.bas", &len);

	ParseTokenVector toks = LexTokensFromString(fileContents, len);
	
	for (int i = 0 ; i < toks.count; i++){
		//printf("Tok: '%.*s'\n", toks.data[i].len, toks.data[i].str);
	}
	
	//printf("------\n");
	LocalLabelVector labels = { 0 };
	ProcDefVector defs = ParseTokens(toks, &labels);

	FILE* gbaOut = fopen("test.gba", "wb");

	for (int i = 0; i < defs.count; i++) {
		printf("Proc %.*s\n", defs.data[i].procName.len, defs.data[i].procName.str);
		LinkLocalLabels(&defs.data[i], labels);
		for (int j = 0; j < defs.data[i].ops.count; j++) {
			ArmInstruction inst = AsmOpToMachineInstruction(defs.data[i].ops.data[j], j);
			printf("\tInst %3s(%2d): 0x%X\n", opCodeNames[defs.data[i].ops.data[j].opCode], j, inst.val);

			//bool isWord = defs.data[i].ops.data[j].opCode == OC_LiteralWord;
			//unsigned int fileInst = isWord ? SwitchEndianness(inst.val) : inst.val;
			//printf("  file: 0x%X\n", fileInst);
			fwrite(&inst.val, 1, 4, gbaOut);
		}
	}

	fclose(gbaOut);
	
	ParseTokenVectorDestroy(&toks);

	ProcDefVectorDestroy(&defs);
	
	free(fileContents);
	
	return 0;
}