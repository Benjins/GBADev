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

//int8 pSample[304*30];

#define SOUND_BUFFER_SIZE 96
int8 soundBuffer[SOUND_BUFFER_SIZE*2];
int8* currBuffer = soundBuffer;

// Timer flags
#define TIMER_ENABLED       0x0080


#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#define MEM_IO   0x04000000
#define MEM_VRAM 0x06000000
#define FRAME_MEM ((volatile uint16*)MEM_VRAM)
#define REG_DISPLAY        (*((volatile uint32 *)(MEM_IO)))

#include "sounds.h"
#include "midi.h"

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	REG_DISPLAY = 0x0403;
	
	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
		FRAME_MEM[i] = 0;
	}
	
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
		VBlankIntrWait();
		sample += SOUND_BUFFER_SIZE;
		
		for(int x = 0; x < SCREEN_WIDTH; x += 3){
			int y = SCREEN_HEIGHT/2 + (currBuffer[x/3]/4);
			FRAME_MEM[x + SCREEN_WIDTH*y] = 0;
			FRAME_MEM[x + SCREEN_WIDTH*y + 1] = 0;
			FRAME_MEM[x + SCREEN_WIDTH*y + 2] = 0;
		}
		
		for(int k = 0; k < SOUND_BUFFER_SIZE; k++){
			currBuffer[k] = 0;
		}
		
		for(int i = 0; i < testSong.length; i++){
			if(testSong.notes[i].start < sample && testSong.notes[i].start + testSong.notes[i].length > sample){
				int offset = sample - testSong.notes[i].start;
				int pitch = testSong.notes[i].pitch;
				for(int k = 0; k < SOUND_BUFFER_SIZE; k++){
					currBuffer[k] += ((offset + k) % pitch > pitch/2) ? 0 : 40;
				}
			}
		}
		
		/*
		for(int i = 0; i < SOUND_BUFFER_SIZE; i++){
			currBuffer[i] = snd.data[soundCursor];
			soundCursor = (soundCursor + 1) % snd.dataLength;
		}
		*/
		
		if(currBuffer == soundBuffer){
			currBuffer += SOUND_BUFFER_SIZE;
			
			REG_DMA1_CNT = 0;
			REG_DMA1_CNT = 0xB640000E;
		}
		else{
			currBuffer = soundBuffer;
		}
	
		for(int x = 0; x < SCREEN_WIDTH; x += 3){
			int y = SCREEN_HEIGHT/2 + (currBuffer[x/3]/4);
			FRAME_MEM[x + SCREEN_WIDTH*y] = 0x7FFF;
			FRAME_MEM[x + SCREEN_WIDTH*y + 1] = 0x7FFF;
			FRAME_MEM[x + SCREEN_WIDTH*y + 2] = 0x7FFF;
		}
	}
	
	return 0;
}
 
