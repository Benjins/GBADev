#ifndef ARM_INST_H
#define ARM_INST_H

#pragma once

typedef struct {
	union {
		unsigned int val;

		struct {
			unsigned int immByte  : 8;
			unsigned int immShift : 4;
			unsigned int _immPacking : 20;
		};

		struct{
			unsigned int shiftedDstReg : 4;
			unsigned int _shiftedDstReserved : 3;
			unsigned int shiftedDstShift : 5;
		};

		struct {
			unsigned int op2 : 12;
			unsigned int regDst : 4;
			unsigned int regOp1 : 4;
			unsigned int setCondFlags : 1;
			unsigned int opCode : 4;
			unsigned int imm : 1;

			unsigned int _instType : 2;
			unsigned int condCode : 4;
		};

		struct {
			unsigned int op2_reserved : 6;
			unsigned int regOp2 : 4;
		};

		struct {
			unsigned int regM : 4;
			unsigned int branching_reserved : 16;
			unsigned int bxBits : 5;
		};

		struct {
			int branchOffset : 24;
			unsigned int branchXL : 4;
			unsigned int _branchCondCode : 4;
		};

		struct {
			unsigned int regBitfield : 16;
			unsigned int ldStrReg : 4;
			unsigned int ldStrOpcode : 5;

			unsigned int instType : 3;
			unsigned int _condCode : 4;
		};
		
		struct {
			unsigned int swiImmediate : 24;
			unsigned int _swiReserved : 4;
			unsigned int _swiCondCode : 4;
		};

		struct {
			unsigned int ldrStrOffset : 12;
			unsigned int _ldrStrPadding : 8;
			unsigned int ldrStrIsLoad : 1;
			unsigned int ldrStrWriteBackPostOffset : 1;
			unsigned int ldrStrByteNotWord : 1;
			unsigned int ldrStrOffsetSign : 1;
			unsigned int ldrStrPreOffset : 1;
			unsigned int _ldrStrPadding2 : 7;
		};

		struct {
			unsigned int ldrStrHalfWordOffsetLowNibble : 4;
			unsigned int ldrStrHalfWordReserved : 4; // set to 1011, aka 0x0B
			unsigned int ldrStrHalfWordOffsetHighNibble : 4;
			unsigned int ldrStrHalfWordDestReg : 4;
			unsigned int ldrStrHalfWordBaseReg : 4;
			unsigned int ldrStrHalfWordIsLoad : 1;
			unsigned int ldrStrHalfWordWriteBack : 1;
			unsigned int ldrStrHalfWordReserved1 : 1; // Set to 1
			unsigned int ldrStrHalfWordOffsetSign : 1;
			unsigned int ldrStrHalfWordPreOffset : 1; // Set to 1
			unsigned int ldrStrHalfWordReserved2 : 3; // Set to 0
			unsigned int ldrStrHalfWordCondCode : 4;
		};
	};
} ArmInstruction;

#define GLUE_TOKS_NOX(a, b) a ## b
#define GLUE_TOKENS(a, b) GLUE_TOKS_NOX(a, b)

#define static_assert(expr) char GLUE_TOKENS(static_assert_reserved, __COUNTER__) [(expr) ? 1024 : -1]

static_assert(sizeof(ArmInstruction) == 4);

#endif
