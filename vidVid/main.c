typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef char int8;
typedef short int16;
typedef int int32;

#define REG_SND_DMGCNT  (*(volatile uint16*)0x4000080)
#define REG_SND_DSCNT   (*(volatile uint16*)0x4000082)
#define REG_SND_STAT    (*(volatile uint16*)0x4000084)   

#define REG_TM0D        (*(volatile uint16*)0x4000100)
#define REG_TM0CNT      (*(volatile uint16*)0x4000102)
#define REG_TM1D        (*(volatile uint16*)0x4000104)
#define REG_TM1CNT      (*(volatile uint16*)0x4000106)
#define REG_TM2D        (*(volatile uint16*)0x4000108)
#define REG_TM2CNT      (*(volatile uint16*)0x400010A)
#define REG_TM3D        (*(volatile uint16*)0x400010C)
#define REG_TM3CNT      (*(volatile uint16*)0x400010E)

#define REG_FIFO_A      (*(volatile uint16*)0x040000A0)
#define REG_FIFO_B      (*(volatile uint16*)0x040000A4)

#define REG_DMA0_SRCADDR (*(volatile uint32*)0x40000B0)
#define REG_DMA0_DSTADDR (*(volatile uint32*)0x40000B4)
#define REG_DMA0_CNT     (*(volatile uint32*)0x40000B8)

#define REG_DMA1_SRCADDR (*(volatile uint32*)0x40000BC)
#define REG_DMA1_DSTADDR (*(volatile uint32*)0x40000C0)
#define REG_DMA1_CNT     (*(volatile uint32*)0x40000C4)

#define REG_DMA3_SRCADDR (*(volatile uint32*)0x40000D4)
#define REG_DMA3_DSTADDR (*(volatile uint32*)0x40000D8)
#define REG_DMA3_CNT     (*(volatile uint32*)0x40000DC)


// DMA flags
#define HALF_WORD_DMA       0x00000000
#define DEST_REG_SAME       0x00400000
#define ENABLE_DMA          0x80000000
#define WORD_DMA            0x04000000
#define START_ON_FIFO_EMPTY 0x30000000
#define DMA_REPEAT          0x02000000

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static inline void  enable_sound(){REG_SND_STAT |= 0x80;}
static inline void disable_sound(){REG_SND_STAT &= ~0x80;}

// Timer flags
#define TIMER_ENABLED       0x0080

#include <gba_interrupt.h>
#include <gba_systemcalls.h>

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#define MEM_IO   0x04000000
#define MEM_VRAM 0x06000000
#define FRAME_MEM ((volatile uint16*)MEM_VRAM)
#define REG_DISPLAY        (*((volatile uint32*)(MEM_IO)))
#define REG_DISPLAY_VCOUNT (*((volatile uint32 *)(MEM_IO + 0x0006)))

#define REG_DISPLAY_STAT (*((volatile uint16*)(MEM_IO + 0x04)))

#include "vid.h"
#include "sounds.h"

#define SOUND_BUFFER_SIZE 96
int8 soundBuffer[SOUND_BUFFER_SIZE*3];
int8* currBuffer = soundBuffer;

void FastSet(const void* src, void* dst, int count){
	
}

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	
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
			FastSet(astley.frames[frameIdx/2], (void*)FRAME_MEM, SCREEN_WIDTH*SCREEN_HEIGHT/2);
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
		
		VBlankIntrWait();
		//asm("swi 0x05");
	}
	
	return 0;
}
 

