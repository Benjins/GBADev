#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8;

#define bool int
#define true 1
#define false 0

#define CONDITIONAL_ASPECT 0xF0000000
#define INST_TYPE_ASPECT   0x0E000000
#define SECOND_OP_ASPECT   0x02000000
#define OPCODE_ASPECT      0x01E00000
#define FLAGS_SET_ASPECT   0x00100000
#define OP1_ASPECT         0x000F0000
#define OP_DST_ASPECT      0x0000F000

#define OP2_REG_SHIFT_ASPECT  0x00000FF0
#define OP2_REG_VALUE_ASPECT  0x0000000F
#define OP2_IMM_SHIFT_ASPECT  0x00000F00
#define OP2_IMM_VALUE_ASPECT  0x000000FF

#define CONDITIONAL_SHIFT 28
#define INST_TYPE_SHIFT   25
#define SECOND_OP_SHIFT   25
#define OPCODE_SHIFT      21
#define FLAGS_SET_SHIFT   20
#define OP1_SHIFT         16
#define OP_DST_SHIFT      12

#define OP2_REG_SHIFT_SHIFT 4
#define OP2_REG_VALUE_SHIFT 0
#define OP2_IMM_SHIFT_SHIFT 8
#define OP2_IMM_VALUE_SHIFT 0


#define LDSTR_OPCODE_ASPECT 0x00F80000
#define LDSTR_OPCODE_SHIFT  20

#define LDSTR_REGN_ASPECT 0x000F0000
#define LDSTR_REGN_SHIFT  16

#define GET_BIT(val, bit) (((val) & (1 << (bit))) >> (bit))

#define GET_BITS(val, low, high) \
(((unsigned int)(((val) >> (low)) << (31 - ((high) - (low))))) >> (31 - ((high) - (low))))

const char* condCodeNames[] = {
	"EQ",
    "NE",
    "CS",
    "CC",
    "MI",
    "PL",
    "VS",
    "VC",
    "HI",
    "LS",
    "GE",
    "LT",
    "GT",
    "LE",
    "AL",
	"XX"
};

const char* opCodeNames[] = {
	"AND",
	"EOR",
	"SUB",
	"RSB",
	"ADD",
	"ADC",
	"SBC",
	"RSC",
	"TST",
	"TEQ",
	"CMP",
	"CMN",
	"ORR",
	"MOV",
	"BIC",
	"MVN"
};

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

int main(int argc, char** argv){
	FILE* asmFile = fopen("main.bin", "rb");

	fseek(asmFile, 0, SEEK_END);
	int fileSize = ftell(asmFile);
	fseek(asmFile, 0, SEEK_SET);

	uint8* fileBuffer = malloc(fileSize);
	fread(fileBuffer, 1, fileSize, asmFile);
	fclose(asmFile);

	int* instPtr = (int*)fileBuffer;
	for (int i = 0; i < fileSize / 4; i++){
		ArmInstruction inst = {};
		inst.val = instPtr[i];
		printf("Inst 0x%04X: 0x%08X(type 0x%02X): ", i, inst.val, inst.instType);
		if ((inst.instType & 0x06) == 0)
		{			
			int opCode = inst.opCode;

			int bxBits = inst.bxBits;
			if (bxBits == 0x12){
				int branchType = inst.branchType;
				int regM = inst.regM;
				if (branchType == 3){
					printf("BLX.%s r%d",
						condCodeNames[inst.condCode],
						regM);
				}
				else if(branchType == 1){
					printf("BX.%s r%d",
						condCodeNames[inst.condCode],
						regM);
				}
				else{
					printf("<Unknown branch type: %d>", branchType);
				}
			}
			else{			
				bool secondOpIsImm = (inst.val & SECOND_OP_ASPECT) != 0;  
				bool setFlags = (inst.val & FLAGS_SET_ASPECT) != 0;
				int lhsReg = (inst.val & OP1_ASPECT) >> OP1_SHIFT; 
				int dstReg = (inst.val & OP_DST_ASPECT) >> OP_DST_SHIFT;

				printf("%s.%s%s r%d, r%d, ", 
						opCodeNames[opCode], 
						condCodeNames[inst.condCode],
						(setFlags ? "_S" : ""),
						dstReg,
						lhsReg);
				if (secondOpIsImm){
					int immShift = (inst.val & OP2_IMM_SHIFT_ASPECT) >> OP2_IMM_SHIFT_SHIFT;
					int immVal = (inst.val & OP2_IMM_VALUE_ASPECT) >> OP2_IMM_VALUE_SHIFT;
					printf("%d", immVal << immShift);
				}
				else{
					int regShift = (inst.val & OP2_REG_SHIFT_ASPECT) >> OP2_REG_SHIFT_SHIFT;
					int regVal = (inst.val & OP2_REG_VALUE_ASPECT) >> OP2_REG_VALUE_SHIFT;
					printf("r%d << %d", regVal, regShift);
				}
			}
		}
		else if ((inst.instType & 0x06) == 0x02){
			int opType  = inst.ldStrOpcode;
			int regN  = inst.ldStrReg;
			
			int opParamA = GET_BIT(inst.val, 25);
			int opParamB = GET_BIT(inst.val,  4);
			
			if (opParamA == 1 && opParamB == 1){
				// TODO: media instructions
			}
			else{
				if ((opType & 0x07) == 0 
				|| ((opType & 0x07) == 2 && (opType && 0x10))){
					if (opParamA == 1){
						int dstReg = inst.regDst;
						int lhsReg = inst.regOp1;
						int offset = GET_BITS(inst.val,  7, 11);
						int regM = inst.regM;
						printf( "STR.%s r%d, [r%d, r%d, #%d]",  
								condCodeNames[inst.condCode],
								dstReg,
								lhsReg,
								regM,
								offset);
					}
					else{
						int dstReg = inst.regDst;
						int lhsReg = inst.regOp1;
						int offset = inst.op2;
						printf( "STR.%s r%d, [r%d, #%d] ; 0x%X",  
								condCodeNames[inst.condCode],
								dstReg,
								lhsReg,
								offset,
								offset);
					}
				}
				else if ((opType & 0x07) == 1 
					 || ((opType & 0x07) == 3 && (opType && 0x10))){
					if (opParamA == 1){
						printf("LDR, a=1");
					}
					else{
						int dstReg = inst.regDst;
						int lhsReg = inst.regOp1;
						int offset = inst.op2;
						printf( "LDR.%s r%d, [r%d, #%d] ; 0x%X",  
								condCodeNames[inst.condCode],
								dstReg,
								lhsReg,
								offset,
								offset);
					}
				}
			}
		}
		else if ((inst.instType & 0x06) == 0x04){
			int opType = GET_BITS(inst.val, 20, 25);
			int regN = GET_BITS(inst.val, 16, 19);
			int r = GET_BIT(inst.val, 15);
			
			if ((opType & 0x30) == 0x20){
				int offset = GET_BITS(inst.val, 0, 23);
				if (offset & (1 << 23)){
					offset = -1 * (~offset & ((1 << 24) - 1));
				}
				//offset *= 4;
				printf( "B.%s 0x%04X",  
							condCodeNames[inst.condCode],
							i + offset + 2);
			}
			else if((opType & 0x30) == 0x30){
				int offset = GET_BITS(inst.val, 0, 23);
				if (offset & (1 << 23)){
					offset = -1 * (~offset & ((1 << 24) - 1));
				}
				//offset *= 4;
				printf( "BL.%s 0x%04X",  
							condCodeNames[inst.condCode],
							i + offset + 2);
			}
			else{
				int regList = GET_BITS(inst.val, 0, 15);
				if (GET_BITS(inst.val, 16, 24) == 0x012D){
					printf("PUSH.%s ", condCodeNames[inst.condCode]);
					PrintRegisterList(stdout, regList);
				}
				else if (GET_BITS(inst.val, 16, 24) == 0x00BD){
					printf("POP.%s ", condCodeNames[inst.condCode]);
					PrintRegisterList(stdout, regList);
				}
				else{
					// TODO: STMIA, LDMIA, etc.
				}
			}
		}
		else if ((inst.instType & 0x06) == 0x06){
			
		}
		else{
			// TODO
		}
		
		printf("\n");
	}
	
	printf("-------------\n");
	
	for (int i = 0; i < fileSize / 4; i++){
		const int inst = instPtr[i];
		int condCode = (inst & CONDITIONAL_ASPECT) >> CONDITIONAL_SHIFT;
		int instType = (inst & INST_TYPE_ASPECT) >> INST_TYPE_SHIFT;
		printf("Inst 0x%04X: 0x%08X(type 0x%02X): ", i, inst, instType);
		if ((instType & 0x06) == 0)
		{			
			int opCode = (inst & OPCODE_ASPECT) >> OPCODE_SHIFT;

			int bxBits = GET_BITS(inst, 20, 24);
			if (bxBits == 0x12){
				int branchType = GET_BITS(inst, 4, 7);
				int regM = GET_BITS(inst, 0, 3);
				if (branchType == 3){
					printf("BLX.%s r%d",
						condCodeNames[condCode],
						regM);
				}
				else if(branchType == 1){
					printf("BX.%s r%d",
						condCodeNames[condCode],
						regM);
				}
				else{
					
				}
			}
			else{			
				bool secondOpIsImm = (inst & SECOND_OP_ASPECT) != 0;  
				bool setFlags = (inst & FLAGS_SET_ASPECT) != 0;
				int lhsReg = (inst & OP1_ASPECT) >> OP1_SHIFT; 
				int dstReg = (inst & OP_DST_ASPECT) >> OP_DST_SHIFT;

				printf("%s.%s%s r%d, r%d, ", 
						opCodeNames[opCode], 
						condCodeNames[condCode],
						(setFlags ? "_S" : ""),
						dstReg,
						lhsReg);
				if (secondOpIsImm){
					int immShift = (inst & OP2_IMM_SHIFT_ASPECT) >> OP2_IMM_SHIFT_SHIFT;
					int immVal = (inst & OP2_IMM_VALUE_ASPECT) >> OP2_IMM_VALUE_SHIFT;
					printf("%d", immVal << immShift);
				}
				else{
					int regShift = (inst & OP2_REG_SHIFT_ASPECT) >> OP2_REG_SHIFT_SHIFT;
					int regVal = (inst & OP2_REG_VALUE_ASPECT) >> OP2_REG_VALUE_SHIFT;
					printf("r%d << %d", regVal, regShift);
				}
			}
		}
		else if ((instType & 0x06) == 0x02){
			int opType  = (inst & LDSTR_OPCODE_ASPECT) >> LDSTR_OPCODE_SHIFT;
			int regN  = (inst & LDSTR_REGN_ASPECT) >> LDSTR_REGN_SHIFT;
			
			int opParamA = GET_BIT(inst, 25);
			int opParamB = GET_BIT(inst,  4);
			
			if (opParamA == 1 && opParamB == 1){
				// TODO: media instructions
			}
			else{
				if ((opType & 0x07) == 0 
				|| ((opType & 0x07) == 2 && (opType && 0x10))){
					if (opParamA == 1){
						int dstReg = GET_BITS(inst, 12, 15);
						int lhsReg = GET_BITS(inst, 16, 19);
						int offset = GET_BITS(inst,  7, 11);
						int regM = GET_BITS(inst, 0, 4);
						printf( "STR.%s r%d, [r%d, r%d, #%d]",  
								condCodeNames[condCode],
								dstReg,
								lhsReg,
								regM,
								offset);
					}
					else{
						int dstReg = GET_BITS(inst, 12, 15);
						int lhsReg = GET_BITS(inst, 16, 19);
						int offset = GET_BITS(inst,  0, 11);
						printf( "STR.%s r%d, [r%d, #%d] ; 0x%X",  
								condCodeNames[condCode],
								dstReg,
								lhsReg,
								offset,
								offset);
					}
				}
				else if ((opType & 0x07) == 1 
					 || ((opType & 0x07) == 3 && (opType && 0x10))){
					if (opParamA == 1){
						printf("LDR, a=1");
					}
					else{
						int dstReg = GET_BITS(inst, 12, 15);
						int lhsReg = GET_BITS(inst, 16, 19);
						int offset = GET_BITS(inst,  0, 11);
						printf( "LDR.%s r%d, [r%d, #%d] ; 0x%X",  
								condCodeNames[condCode],
								dstReg,
								lhsReg,
								offset,
								offset);
					}
				}
			}
		}
		else if ((instType & 0x06) == 0x04){
			int opType = GET_BITS(inst, 20, 25);
			int regN = GET_BITS(inst, 16, 19);
			int r = GET_BIT(inst, 15);
			
			if ((opType & 0x30) == 0x20){
				int offset = GET_BITS(inst, 0, 23);
				if (offset & (1 << 23)){
					offset = -1 * (~offset & ((1 << 24) - 1));
				}
				//offset *= 4;
				printf( "B.%s 0x%04X",  
							condCodeNames[condCode],
							i + offset + 2);
			}
			else if((opType & 0x30) == 0x30){
				int offset = GET_BITS(inst, 0, 23);
				if (offset & (1 << 23)){
					offset = -1 * (~offset & ((1 << 24) - 1));
				}
				//offset *= 4;
				printf( "BL.%s 0x%04X",  
							condCodeNames[condCode],
							i + offset + 2);
			}
			else{
				int regList = GET_BITS(inst, 0, 15);
				if (GET_BITS(inst, 16, 24) == 0x012D){
					printf("PUSH.%s ", condCodeNames[condCode]);
					PrintRegisterList(stdout, regList);
				}
				else if (GET_BITS(inst, 16, 24) == 0x00BD){
					printf("POP.%s ", condCodeNames[condCode]);
					PrintRegisterList(stdout, regList);
				}
				else{
					// TODO: STMIA, LDMIA, etc.
				}
			}
		}
		else if ((instType & 0x06) == 0x06){
			
		}
		else{
			// TODO
		}
		
		printf("\n");
	}
		
	free(fileBuffer);

	return 0;
}