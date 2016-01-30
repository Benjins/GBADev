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

int8 pSample[4000];

// Timer flags
#define TIMER_ENABLED       0x0080

// our Timer interval that we calculated earlier (note that this
// value depends on our playback frequency and is therefore not set in
// stone)
#define TIMER_INTERVAL      (0xFFFF - 761)

#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>

#define MEM_IO   0x04000000
#define MEM_VRAM 0x06000000
#define FRAME_MEM ((volatile uint16*)MEM_VRAM)
#define REG_DISPLAY        (*((volatile uint32 *)(MEM_IO)))

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	REG_DISPLAY = 0x0403;
	
	int pitch = 6;
	for(int i = 0; i < 4000; i++){
		pSample[i] = (i % pitch) >= (pitch/2) ? 120 : -120;
		
		if(i % 100 == 0){
			pitch++;
			if(pitch >= 10){
				pitch = 6;
			}
		}
	}
	
	
	enable_sound();
	
	REG_SND_DMGCNT = 0x3F;
	
	REG_SND_DSCNT = 0x0606 | (1 << 8) | (1 << 9);
	REG_TM0D = 65536 - (16777216 / 1000);
	REG_TM0CNT = 0x80;
	REG_TM1D = 65536 - (16777216 / 1000);
	REG_TM1CNT = 0x80;
	REG_DMA1_SRCADDR = (uint32) pSample;
	REG_DMA1_DSTADDR = 0x040000A0;
	REG_DMA1_CNT = 0xB640000E;
	
	
	for(int i = 0; i < 200; i++){
		FRAME_MEM[240+i] = 0x2CC5;
	}
	
	int idx = 0;
	while(1){
		VBlankIntrWait();
		idx++;
		
		for(int j = 0; j < 61; j++){
			FRAME_MEM[j+240*5] = 0;
		}
		for(int j = 0; j < idx; j++){
			FRAME_MEM[j+240*5] = 0x7FFF;
		}
		
		if(idx >= 60){
			idx = 0;
			
			/*
			pitch = 20;
			for(int i = 0; i < 4000; i++){
				pSample[i] = (i % pitch) >= (pitch/2) ? 120 : -120;
				
				if(i % 100 == 0){
					pitch++;
					if(pitch >= 30){
						pitch = 20;
					}
				}
			}
			*/
			
			
			REG_DMA1_CNT = 0;
			
			
			REG_DMA1_SRCADDR = (uint32) pSample;
			REG_DMA1_DSTADDR = 0x040000A0;
			REG_DMA1_CNT = 0xB640000E;
			
			idx = 0;
		}
	}
	
	return 0;
}
 

