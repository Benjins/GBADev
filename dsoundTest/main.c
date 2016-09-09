#include "../common/gba.h"

#define SOUND_BUFFER_SIZE 304
int8 soundBuffer[SOUND_BUFFER_SIZE*3];
int8* currBuffer = soundBuffer;

#include "sounds.h"
#include "midi.h"

#define FRAME_MEM ((volatile uint16*)MEM_VRAM)

int main(void) {
	INT_VECTOR = InterruptMain;
	
	BNS_REG_IME	= 0;
	REG_DISPSTAT |= LCDC_VBL;
	BNS_REG_IE |= IRQ_VBLANK;
	BNS_REG_IME	= 1;
	
	REG_DISPLAY = 0x0403;
	
	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
		FRAME_MEM[i] = 0;
	}
	
	enable_sound();
	
	REG_SND_DMGCNT = 0x3F;
	
	REG_SND_DSCNT = 0x0606 | (1 << 8) | (1 << 9);
	REG_TM1D = 65536 - (16777216 / 18157);
	REG_TM1CNT = 0x80;
	REG_DMA1_SRCADDR = (uint32) soundBuffer;
	REG_DMA1_DSTADDR = 0x040000A0;
	REG_DMA1_CNT = 0xB640000E;
	
	int soundCursor = 0;
	int sample = 0;
	
	while(1){
		
		for(int x = 0; x < SCREEN_WIDTH; x += 3){
			int y = SCREEN_HEIGHT/2 + (currBuffer[x/3]);
			FRAME_MEM[x + SCREEN_WIDTH*y] = 0x7FFF;
			FRAME_MEM[x + SCREEN_WIDTH*y + 1] = 0x7FFF;
			FRAME_MEM[x + SCREEN_WIDTH*y + 2] = 0x7FFF;
		}
		
		for(int k = 0; k < SOUND_BUFFER_SIZE; k++){
			currBuffer[k] = snd.data[sample+k];
		}
		
		sample += SOUND_BUFFER_SIZE;
		
		if (sample > snd.dataLength){
			sample -= snd.dataLength;
		}
		
		if(currBuffer == soundBuffer){
			currBuffer = soundBuffer + SOUND_BUFFER_SIZE;
			
			REG_DMA1_CNT = 0;
			REG_DMA1_CNT = 0xB640000E;
		}
		else if(currBuffer == soundBuffer + SOUND_BUFFER_SIZE){
			currBuffer = soundBuffer + (2*SOUND_BUFFER_SIZE);
		}
		else{
			currBuffer = soundBuffer;
		}
		
		asm("swi 0x05");

		for(int x = 0; x < SCREEN_WIDTH; x += 3){
			int y = SCREEN_HEIGHT/2 + (currBuffer[x/3]);
			FRAME_MEM[x + SCREEN_WIDTH*y] = 0;
			FRAME_MEM[x + SCREEN_WIDTH*y + 1] = 0;
			FRAME_MEM[x + SCREEN_WIDTH*y + 2] = 0;
		}
		
	}
	
	return 0;
}
 

