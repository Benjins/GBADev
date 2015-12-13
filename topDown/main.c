#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef char int8;
typedef short int16;
typedef int int32;

typedef uint16 rgb15;
typedef struct object_attributes {
	uint16 attribute_zero;
	uint16 attribute_one;
	uint16 attribute_two;
	uint16 pad;
} __attribute__((aligned(4))) object_attributes;
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
#define tile_memory ((volatile tile_block *)MEM_VRAM)
#define object_palette_memory ((volatile rgb15 *)(MEM_PAL + 0x200))
#define bg0_palette_memory ((volatile rgb15 *)(MEM_PAL))

typedef uint16 SCREENBLOCK[1024];

#define scr_blk_mem          ((SCREENBLOCK*)MEM_VRAM)

#define 	BG_REG_64x64   0xC000
#define 	BG_REG_64x32   0x4000

#define 	BG_CBB(n)   ((n)<<2)
#define 	BG_SBB(n)   ((n)<<8)

#define REG_BG0_CNT       (*(volatile uint16*)(MEM_IO+0x0008))
#define REG_BG0_OFS      ((volatile int16*)(MEM_IO+0x0010))

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

typedef struct {
	int16 position[2];
} __attribute__((aligned(4))) object;

#define MAX_OBJECT_COUNT 10
object objects[MAX_OBJECT_COUNT];

int objectCount = 0;

void AddObject(int x, int y){
	objects[objectCount].position[0] = x;
	objects[objectCount].position[1] = y;
	
	objectCount++;
}

volatile object_attributes* playerAttribs = &oam_memory[0];
volatile object_attributes* objectAttribs = &oam_memory[1];

// Form a 16-bit BGR GBA colour from three component values
static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

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

int main(void) {
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		bg0_palette_memory[i] = paletteColors[i];
	}
	
	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }
	
	volatile uint16* player_tile_memory = (uint16 *)tile_memory[4][1];
	set_sprite_memory(playerSprite, player_tile_memory);
	
	volatile uint16* object_tile_memory = (uint16 *)tile_memory[4][2];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { object_tile_memory[i] = 0x2232; }
	
	Sprite font[] = {aFont, bFont, cFont, dFont, eFont, fFont, gFont, hFont, iFont,
					jFont, kFont, lFont, mFont, nFont, oFont, pFont, qFont, rFont, sFont,
					tFont, uFont, vFont, wFont, xFont, yFont, zFont};
	
	for(int i = 0; i < ARRAY_LENGTH(font); i++){
		volatile uint16* uiFontMemory = (uint16 *)tile_memory[4][14+i];
		set_sprite_memory(font[i], uiFontMemory);
	}
	
	// Write the colour palette for our sprites into the first palette of
	// 16 colours in colour palette memory (this palette has index 0).
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		object_palette_memory[i] = paletteColors[i];
	}
	
	int playerX = 0, playerY = 0;
	
	const int player_height = 8, player_width = 8;
	
	const int centerX = SCREEN_WIDTH/2 - player_height/2;
	const int centerY = SCREEN_HEIGHT/2 - player_width/2;
	
	playerAttribs->attribute_zero = 0; 
	playerAttribs->attribute_one = 0; 
	playerAttribs->attribute_two = 1;
	set_object_position(playerAttribs, centerX, centerY);
	
	for(int i = 0; i < MAX_OBJECT_COUNT; i++){
		volatile object_attributes* objectAttrib = &objectAttribs[i];
		objectAttrib->attribute_zero = 0; 
		objectAttrib->attribute_one = 0; 
		objectAttrib->attribute_two = 2;
		set_object_position(objectAttrib, -10, -10);
	}
	
	AddObject(-50, 50);
	AddObject(-20, 150);
	AddObject(210, -20);
	AddObject(120, 70);
	AddObject(180, 90);
	AddObject(70, 120);
		
	// Set the display parameters to enable objects, and use a 1D object->tile mapping, and enable BG0
	REG_DISPLAY = 0x1000 | 0x0040 | 0x0100;
	
	REG_BG0_CNT = BG_CBB(0) | BG_SBB(20) | BG_REG_64x32;
	
	REG_BG0_OFS[0] = 0;
	REG_BG0_OFS[1] = 0;
	
	{
		Sprite bgSprites[] = {bg1Sprite,bg2Sprite,bg3Sprite};
		
		for(int i = 0; i < 3; i++){
			volatile uint16* bg_tile_mem = (uint16 *)tile_memory[0][i];
			
			set_sprite_memory(bgSprites[i],bg_tile_mem);
		}
	}
	
	volatile uint16* screenmapStart = &scr_blk_mem[20][0];
	for(int i = 0; i < backMap.width*backMap.height; i++){
		screenmapStart[i] = backMap_data[i];
	}
	
	uint32 prevKeys = 0;
	
	while (1) {
		VBlankIntrWait();
		
		uint32 key_states = ~REG_KEY_INPUT & KEY_ANY;
		
		int player_max_clamp_y = SCREEN_HEIGHT - player_height;
		int player_max_clamp_x = SCREEN_WIDTH - player_width;
		
		if (key_states & KEY_LEFT) { playerX--;}
		if (key_states & KEY_RIGHT) { playerX++;}
		if (key_states & KEY_UP) { playerY--;}
		if (key_states & KEY_DOWN) { playerY++;}
		
		REG_BG0_OFS[0] = playerX;
		REG_BG0_OFS[1] = playerY;
			
		for(int i = 0; i < objectCount; i++){
			volatile object_attributes* objectAttrib = &objectAttribs[i];
			int screenX = objects[i].position[0] - playerX + centerX;
			int screenY = objects[i].position[1] - playerY + centerY;
			
			//Prevent wrap-around from the truncation that set_pos uses.
			//TODO: Use clamp instead?
			if(screenX < -10 || screenX > SCREEN_WIDTH  + 10
			|| screenY < -10 || screenY > SCREEN_HEIGHT + 10){
				set_object_position(objectAttrib, -10, -10);
			}
			else{
				set_object_position(objectAttrib, screenX, screenY);
			}
		}
		
		prevKeys = key_states;
		
	}

	return 0;
}