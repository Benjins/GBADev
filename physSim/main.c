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
#define REG_DISPSTAT       (*((volatile uint16 *)(MEM_IO + 0x0004)))
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

#include "fixed.c"

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

typedef void ( * IntFn)(void);

#define INT_VECTOR	*(IntFn *)(0x03007ffc)

#define BNS_REG_IE  (*(volatile uint16*)0x04000200)
#define BNS_REG_IF  (*(volatile uint16*)0x04000202)
#define BNS_REG_IME (*(volatile uint16*)0x04000208)

extern void InterruptMain() __attribute__((section(".iwram")));

#define LCDC_VBL (1 << 3)
#define IRQ_VBLANK (1 << 0)

int main(void) {
	INT_VECTOR = InterruptMain;
	
	BNS_REG_IME	= 0;
	REG_DISPSTAT |= LCDC_VBL;
	BNS_REG_IE |= IRQ_VBLANK;
	BNS_REG_IME	= 1;
	
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		object_palette_memory[i] = paletteColors[i];
		bg0_palette_memory[i] = paletteColors[i];
	}
	
	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }
	
	// Set the display parameters to enable objects, and use a 1D object->tile mapping, and enable BG2
	REG_DISPLAY = 0x1000 | 0x0040 | 0x0700 | 0x0001;
	
	volatile uint16* ballMem = (uint16 *)tile_memory[4][1];
	set_sprite_memory(ballSprite, ballMem);
	
	fixed alpha[5] = {0};
	fixed rotVel[5] = {0};
	
	fixed xPos[5] = {};
	fixed yPos[5] = {};
	
	fixed xVel[5] = {};
	fixed yVel[5] = {};
	
	for(int i = 0; i < 5; i++){
		xPos[i] = makeFixed(30 + i * 18);
		yPos[i] = makeFixed(50 + i * 2);
		
		xVel[i] = 122 * i;
		yVel[i] = 200 * i - 230;
		
		alpha[i] = makeFixed(20 * i + 14);
		rotVel[i] = makeFixed(10 * i - 100);
		
		volatile object_attributes* playerAttribs = &oam_memory[i];
		playerAttribs->attribute_zero = (30 + i * 18) | (1 << 8) | (1 << 9); 
		playerAttribs->attribute_one = (50 + i * 2) | 0x4000 | (i << 9); 
		playerAttribs->attribute_two = 1;
		
		aff_memory[i].pa = 256;
		aff_memory[i].pb = 0;
		aff_memory[i].pc = 0;
		aff_memory[i].pd = 256;
	}
	
	int playerIndex = 0;
	uint32 prevKeys = 0;
	
	const fixed ballSizeInt = 16;
	const fixed ballSize = makeFixed(ballSizeInt);
	
	while (1) {
		//VBlankIntrWait();
		asm("swi 0x05" ::: "r0", "r1", "r2", "r3");
		
		uint32 key_states = ~REG_KEY_INPUT & KEY_ANY;
		
		for(int i = 0; i < 5; i++){
			xPos[i] += xVel[i];
			yPos[i] += yVel[i];
			
			if (xPos[i] <= ballSize/2 || xPos[i] >= makeFixed(SCREEN_WIDTH) - ballSize/2){
				xVel[i] *= -1;
			}
			
			if (yPos[i] <= ballSize/2 || yPos[i] >= makeFixed(SCREEN_HEIGHT) - ballSize/2){
				yVel[i] *= -1;
			}
			
			alpha[i] += rotVel[i]/60;
			
			if(alpha[i] >= makeFixed(360)){
				alpha[i] -= makeFixed(360);
			}
			
			if(alpha[i] < 0){
				alpha[i] += makeFixed(360);
			}
			
			fixed sinA = mySin(alpha[i]) >> 2;
			fixed cosA = myCos(alpha[i]) >> 2;
		
			volatile aff_object_attributes* playerAffAttribs = &aff_memory[i];
			playerAffAttribs->pa = (int16)cosA;
			playerAffAttribs->pb = (int16)-sinA;
			playerAffAttribs->pc = (int16)sinA;
			playerAffAttribs->pd = (int16)cosA;
			
			volatile object_attributes* playerAttribs = &oam_memory[i];
			set_object_position(playerAttribs, truncFixedToInt(xPos[i]) - ballSizeInt, truncFixedToInt(yPos[i]) - ballSizeInt);
		}
		
		if (key_states & BUTTON_A){
			alpha[playerIndex] += FIXED_ONE/2;
		}
		if (key_states & BUTTON_B){
			alpha[playerIndex] -= FIXED_ONE/2;
		}
		
		if (key_states & KEY_UP){
			yPos[playerIndex] -= makeFixed(1);
		}
		
		if (key_states & KEY_DOWN){
			yPos[playerIndex] += makeFixed(1);
		}
		
		if (key_states & KEY_LEFT){
			xPos[playerIndex] -= makeFixed(1);
		}
		
		if (key_states & KEY_RIGHT){
			xPos[playerIndex] += makeFixed(1);
		}
		
		if (!(key_states & BUTTON_SELECT) && (prevKeys & BUTTON_SELECT)){
			playerIndex = (playerIndex + 1) % 5;
		}

		prevKeys = key_states;
		
	}

	return 0;
}