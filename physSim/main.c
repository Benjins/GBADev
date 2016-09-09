#include "../common/gba.h"
#include "../common/fixed.c"

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