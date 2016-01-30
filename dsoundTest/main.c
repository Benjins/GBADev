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

/*
REG_DMAxSAD	source	0400:00B0h + 0Ch·x
REG_DMAxDAD	destination	0400:00B4h + 0Ch·x
REG_DMAxCNT	control	0400:00B8h + 0Ch·x
*/

#define REG_DMA0_SRCADDR (*(volatile uint32*)0x40000B0)
#define REG_DMA0_DSTADDR (*(volatile uint32*)0x40000B4)
#define REG_DMA0_CNT     (*(volatile uint32*)0x40000B8)

#define REG_DMA1_SRCADDR (*(volatile uint32*)0x40000BC)
#define REG_DMA1_DSTADDR (*(volatile uint32*)0x40000C0)
#define REG_DMA1_CNT     (*(volatile uint32*)0x40000C4)

#define ENABLE_DMA          0x80000000
#define WORD_DMA            0x04000000
#define START_ON_FIFO_EMPTY 0x30000000
#define DMA_REPEAT          0x02000000

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static inline void  enable_sound(){REG_SND_STAT |= 0x80;}
static inline void disable_sound(){REG_SND_STAT &= ~0x80;}

int8 pSample[1024*20];


// DMA channel 1 register definitions
#define REG_DMA1SAD         *(uint32*)0x40000BC  // source address
#define REG_DMA1DAD         *(uint32*)0x40000C0  // destination address
#define REG_DMA1CNT         *(uint32*)0x40000C4  // control register

// DMA flags
#define WORD_DMA            0x04000000
#define HALF_WORD_DMA       0x00000000
#define ENABLE_DMA          0x80000000
#define START_ON_FIFO_EMPTY 0x30000000
#define DMA_REPEAT          0x02000000
#define DEST_REG_SAME       0x00400000

// Timer 0 register definitions
//#define REG_TM0D            *(uint16*)0x4000100
//#define REG_TM0CNT          *(uint16*)0x4000102

// Timer flags
#define TIMER_ENABLED       0x0080

// FIFO address defines
//#define REG_FIFO_A          0x040000A0
//#define REG_FIFO_B          0x040000A4

// our Timer interval that we calculated earlier (note that this
// value depends on our playback frequency and is therefore not set in
// stone)
#define TIMER_INTERVAL      (0xFFFF - 761)

#include <gba_interrupt.h>

int main(void){
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	int pitch = 10;
	for(int i = 0; i < 1024*20; i++){
		pSample[i] = (i % pitch) >= (pitch/2) ? 120 : -120;
		
		if(i % 100 == 0){
			pitch++;
			if(pitch >= 20){
				pitch = 10;
			}
		}
	}
	
	enable_sound();
	
	REG_SND_DMGCNT = 0x3F;
	
	REG_SND_DSCNT = 0x0606;
	REG_TM0D = 65536 - (16777216 / 1000);
	REG_TM0CNT = 0x80;
	REG_TM1D = 65536 - (16777216 / 1000);
	REG_TM1CNT = 0x80;
	REG_DMA1_SRCADDR = (uint32) pSample;
	REG_DMA1_DSTADDR = 0x040000A0;
	REG_DMA1_CNT = 0xB640000E;
	
	int8* ptr = (int8*)0x040000A0;
	
	int idx = 0;
	while(1){
		//asm("swi 0x05");
		//ptr[idx % 4] = (idx % 100 + 20);
		idx++;
	}
	
	return 0;
}
 

