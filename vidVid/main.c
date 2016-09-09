#include "../common/gba.h"

#include "vid.h"
#include "sounds.h"

#define SOUND_BUFFER_SIZE 96
int8 soundBuffer[SOUND_BUFFER_SIZE*3];
int8* currBuffer = soundBuffer;

void CpuFastSet( const void *src,  void *dst, uint32 mode){
	asm("swi 0x05");
}

#define FRAME_MEM ((volatile uint16*)MEM_VRAM)

int main(void) {
	INT_VECTOR = InterruptMain;
	
	BNS_REG_IME	= 0;
	REG_DISPSTAT |= LCDC_VBL;
	BNS_REG_IE |= IRQ_VBLANK;
	BNS_REG_IME	= 1;
	
	REG_DISPLAY = 0x0403;
	
	int zeroFill[8] = {0};
	CpuFastSet(zeroFill, (void*)FRAME_MEM, (SCREEN_WIDTH*SCREEN_HEIGHT/2) | (1 << 26));
	
	int frameIdx = 0;
	
	enable_sound();
	
	REG_SND_DMGCNT = 0x3F;
	
	REG_SND_DSCNT = 0x0606 | (1 << 8) | (1 << 9);
	REG_TM1D = 65536 - (16777216 / 5734);
	REG_TM1CNT = 0x80;
	REG_DMA1_SRCADDR = (uint32) soundBuffer;
	REG_DMA1_DSTADDR = 0x040000A0;
	REG_DMA1_CNT = 0xB640000E;
	
	int soundCursor = 0;
	int sample = 0;
	
	while(1){
		//FRAME_MEM[frameIdx] = 0xFFFF;
		
		/*
		for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
			FRAME_MEM[i] = astley.frames[frameIdx][i];
		}*/
		
		if(frameIdx %2 == 0){
			CpuFastSet(astley.frames[frameIdx/2], (void*)FRAME_MEM, SCREEN_WIDTH*SCREEN_HEIGHT/2);
		}
		
		frameIdx++;
		frameIdx = frameIdx % (astley.frameCount * 2);
		
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
		
		//VBlankIntrWait();
		asm("swi 0x05");
	}
	
	return 0;
}
 

