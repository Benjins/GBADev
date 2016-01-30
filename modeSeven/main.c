#include <gba_interrupt.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef char int8;
typedef short int16;
typedef int int32;

typedef uint16 rgb15;
typedef struct {
	uint16 attribute_zero;
	uint16 attribute_one;
	uint16 attribute_two;
	uint16 pad;
} __attribute__((aligned(4))) object_attributes;

typedef struct{
    uint16 fill0[3];
    int16 pa;
    uint16 fill1[3];
    int16 pb;
    uint16 fill2[3];
    int16 pc;
    uint16 fill3[3];
    int16 pd;
} __attribute__((aligned(4))) aff_object_attributes;

typedef uint32 tile4bpp[8];
typedef tile4bpp tile_block[512];

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#define MEM_IO   0x04000000
#define MEM_PAL  0x05000000
#define MEM_VRAM 0x06000000
#define MEM_OAM  0x07000000

#define REG_DISPLAY        (*((volatile uint32 *)(MEM_IO)))
#define REG_DISPLAY_VCOUNT (*((volatile uint32 *)(MEM_IO + 0x0006)))
#define REG_KEY_INPUT      (*((volatile uint32 *)(MEM_IO + 0x0130)))

#define KEY_UP     0x0040
#define KEY_DOWN   0x0080
#define KEY_LEFT   0x0020
#define KEY_RIGHT  0x0010
#define KEY_ANY    0x03FF
#define BUTTON_A 0x0001
#define BUTTON_B 0x0002
#define BUTTON_SELECT 0x0004
#define BUTTON_START 0x0008

#define OBJECT_ATTRIBUTE_ZERO_Y_MASK  0xFF
#define OBJECT_ATTRIBUTE_ONE_X_MASK  0x1FF
#define OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK  0x1000

#define oam_memory ((volatile object_attributes *)MEM_OAM)
#define aff_memory ((volatile aff_object_attributes*)MEM_OAM)
#define tile_memory ((volatile tile_block *)MEM_VRAM)
#define object_palette_memory ((volatile rgb15 *)(MEM_PAL + 0x200))
#define bg0_palette_memory ((volatile rgb15 *)(MEM_PAL))
#define bg2_palette_memory ((volatile rgb15 *)(MEM_PAL + 0x100))

typedef uint16 SCREENBLOCK[1024];

typedef struct{
    int16 ma, mb;
    int16 mc, md;
    int32 dx, dy;
} __attribute__((aligned(4))) BG_AFFINE;

#define REG_BG_AFFINE   ((BG_AFFINE*)REG_DISPLAY)

#define scr_blk_mem          ((SCREENBLOCK*)MEM_VRAM)

#define 	BG_REG_64x64   0xC000
#define 	BG_REG_64x32   0x4000
#define     BG_AFF_32x32   0x4000

#define 	BG_CBB(n)   ((n)<<2)
#define 	BG_SBB(n)   ((n)<<8)

#define REG_BG0_CNT      (*(volatile uint16*)(MEM_IO+0x0008))
#define REG_BG0_OFS      ((volatile int16*)(MEM_IO+0x0010))

#define REG_BG1_CNT      (*(volatile uint16*)(MEM_IO+0x000A))
#define REG_BG1_OFS      ((volatile int16*)(MEM_IO+0x0014))

#define REG_BG2_CNT      (*(volatile uint16*)(MEM_IO+0x000C))
#define REG_BG2_OFS      ((volatile int16*)(MEM_IO+0x0018))

#define REG_BG2_AFF_MATR (*(BG_AFFINE*)(MEM_IO + 0x0020))

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

// Form a 16-bit BGR GBA colour from three component values
static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

#define FIXED_DECIMAL 8
#define FIXED_ONE (1 << FIXED_DECIMAL)
typedef int32 fixed;

static inline fixed makeFixed(int x){
	return x << FIXED_DECIMAL;
}

#define fixedFromFlt(x) ((fixed)((x) * FIXED_ONE))

static const fixed deg2rad = fixedFromFlt(3.1415926536f/180);

static inline fixed fixMult(fixed a, fixed b){
	return (a * b) >> FIXED_DECIMAL;
}

static inline fixed fixDiv(fixed a, fixed b){
	return ((a/b) << FIXED_DECIMAL) + ((a%b) << FIXED_DECIMAL) / b;
}

static inline fixed fixPow(fixed b, int e){
	fixed answer = FIXED_ONE;
	for(int i = 0; i < e; i++){
		answer = fixMult(answer, b);
	}
	
	return answer;
}

static inline fixed mySin(fixed in){
	
	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return in - fixPow(in,3)/6 + fixPow(in,5)/120 - fixPow(in,7)/5040 + fixPow(in,9)/362880;
}

static inline fixed myCos(fixed in){

	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return FIXED_ONE - fixMult(in,in)/4 + fixPow(in,4)/24 - fixPow(in,6)/720 + fixPow(in,8)/40320;
}

// Set the position of an object to specified x and y coordinates
static inline void set_object_position(volatile object_attributes *object, int x, int y) {
	object->attribute_zero = (object->attribute_zero & ~OBJECT_ATTRIBUTE_ZERO_Y_MASK) | (y & OBJECT_ATTRIBUTE_ZERO_Y_MASK);
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_X_MASK) | (x & OBJECT_ATTRIBUTE_ONE_X_MASK);
}

static inline void set_object_horizontal_flip(volatile object_attributes *object, int isFlip){
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK) | (isFlip ? OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK : 0);
}

// Clamp 'value' in the range 'min' to 'max' (inclusive).
static inline int clamp(int value, int min, int max) { return (value < min ? min : (value > max ? max : value)); }

static inline int abs(int value) { return (value >= 0 ? value : -value); }

static inline int signum(int value) { return (value > 0 ? 1 : (value < 0 ? -1 : 0) ); }

static inline float clampf(float value, float min, float max) { return (value < min ? min : (value > max ? max : value)); }

#include "assets.h"

static inline void set_sprite_memory(Sprite sprite, volatile uint16* memory){
	int memSize = (sprite.height * sprite.width) / 4;
	for(int i = 0; i < memSize; i++){
		uint16 mem = 0;
		for(int pix = 0; pix < 4; pix++){
			uint16 pixelCol = sprite.data[i*4+pix];
			mem |= (pixelCol << (pix*4));
		}
		memory[i] = mem;
	}
}

/*
0x0f: ObjAffineSet
Input:
r0: source address
r1: destination address
r2: number of calculations
r3: Offset of P matrix elements (2 for bgs, 8 for objects)
*/



int main(void) {
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		object_palette_memory[i] = paletteColors[i];
		bg0_palette_memory[i] = paletteColors[i];
	}
	
	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }
	
	// Set the display parameters to enable objects, and use a 1D object->tile mapping, and enable BG2
	REG_DISPLAY = 0x1000 | 0x0040 | 0x0700 | 0x0001;
	
	REG_BG2_CNT = BG_CBB(0) | BG_SBB(9) | BG_AFF_32x32;
	//REG_BG0_CNT = BG_CBB(0) | BG_SBB(8) | BG_AFF_32x32;
	
	for(int i = 0; i < backMap.bgCount; i++){
		volatile uint16* bg0_tile_mem = (uint16 *)tile_memory[0][i+1];
		volatile uint16* bg1_tile_mem = (uint16 *)tile_memory[1][i+1];
		volatile uint16* bg2_tile_mem = (uint16 *)tile_memory[2][i+1];
	
		set_sprite_memory(backMap.bgSprites[i], bg0_tile_mem);
		set_sprite_memory(backMap.bgSprites[i], bg1_tile_mem);
		set_sprite_memory(backMap.bgSprites[i], bg2_tile_mem);
	}
	
	volatile uint16* screenmap0Start = &scr_blk_mem[9][0];
	
	for(int j = 0; j < 32; j++){
		for(int i = 0; i < 32; i++){
			int backMapIdx = j*backMap.map.width+i;
			
			int scrMapIdx = j*32+i;
			screenmap0Start[scrMapIdx] = backMap.map.data[backMapIdx];
		}
	}
	
	BG_AFFINE bg_aff_default = { 256, 0, 0, 256, 0, 0};

	// Initialize affine registers for bg 2
	REG_BG2_AFF_MATR = bg_aff_default;
	
	uint32 prevKeys = 0;
	
	int xOff = 0;
	
	for(int i = 0; i < 16;  i++){
		volatile uint16* monster_tile_memory = (uint16 *)tile_memory[4][i+2];
		set_sprite_memory(backMap_bg3, monster_tile_memory);
	}
	
	volatile object_attributes* playerAttribs = &oam_memory[0];
	
	playerAttribs->attribute_zero = 50 | (1 << 8) | (1 << 9); 
	playerAttribs->attribute_one = 50 | 0x8000; 
	playerAttribs->attribute_two = 2;
	
	volatile aff_object_attributes* playerAffAttribs = &aff_memory[0];
	
	fixed alpha = 0;
	
	while (1) {
		//VBlankIntrWait();
		asm("swi 0x05");
		
		fixed sinA = mySin(alpha);
		fixed cosA = myCos(alpha);
		
		
		if(alpha > 180){
			sinA *= -1;
		}
		
		if(alpha > 90 && alpha < 270){
			cosA *= -1;
		}
		
		
		playerAffAttribs->pa = (int16)cosA;
		playerAffAttribs->pb = (int16)-sinA;
		playerAffAttribs->pc = (int16)sinA;
		playerAffAttribs->pd = (int16)cosA;
		
		alpha += FIXED_ONE/6;
		
		if(alpha >= makeFixed(360)){
			alpha -= makeFixed(360);
		}
		
		bg_aff_default.ma++;
		bg_aff_default.md++;
		REG_BG2_AFF_MATR = bg_aff_default;
		
		uint32 key_states = ~REG_KEY_INPUT & KEY_ANY;
		
		//REG_BG2_OFS[0] = xOff;
		
		xOff++;
		
		prevKeys = key_states;
		
	}

	return 0;
}