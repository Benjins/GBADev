typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef char int8;
typedef short int16;
typedef int int32;

typedef struct object_attributes {
	uint16 attribute_zero;
	uint16 attribute_one;
	uint16 attribute_two;
	uint16 pad;
} object_attributes __attribute__((aligned(4))) ;
typedef uint32 tile4bpp[8];
typedef tile4bpp tile_block[512];

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

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

typedef uint16 rgb15;

// Form a 16-bit BGR GBA colour from three component values
static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

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
#define BUTTON_R 0x0100
#define BUTTON_L 0x0200

#define OBJECT_ATTRIBUTE_ZERO_Y_MASK  0xFF
#define OBJECT_ATTRIBUTE_ONE_X_MASK  0x1FF
#define OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK  0x1000
#define OBJECT_ATTRIBUTE_ONE_VERTICAL_FLIP_MASK  0x2000

#define oam_memory ((volatile object_attributes *)MEM_OAM)
#define aff_memory ((volatile aff_object_attributes*)MEM_OAM)
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

#define REG_BG1_CNT       (*(volatile uint16*)(MEM_IO+0x000A))
#define REG_BG1_OFS      ((volatile int16*)(MEM_IO+0x0014))

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

// Interrupts
typedef void ( * IntFn)(void);

#define INT_VECTOR	*(IntFn *)(0x03007ffc)

#define BNS_REG_IE  (*(volatile uint16*)0x04000200)
#define BNS_REG_IF  (*(volatile uint16*)0x04000202)
#define BNS_REG_IME (*(volatile uint16*)0x04000208)

extern void InterruptMain() __attribute__((section(".iwram")));

#define LCDC_VBL (1 << 3)
#define IRQ_VBLANK (1 << 0)

// DMA flags
#define HALF_WORD_DMA       0x00000000
#define DEST_REG_SAME       0x00400000
#define ENABLE_DMA          0x80000000
#define WORD_DMA            0x04000000
#define START_ON_FIFO_EMPTY 0x30000000
#define DMA_REPEAT          0x02000000

static inline void  enable_sound(){REG_SND_STAT |= 0x80;}
static inline void disable_sound(){REG_SND_STAT &= ~0x80;}

// Timer flags
#define TIMER_ENABLED       0x0080

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#define MEM_IO   0x04000000
#define MEM_VRAM 0x06000000

#define REG_DISPLAY_STAT (*((volatile uint16*)(MEM_IO + 0x04)))

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

// Clamp 'value' in the range 'min' to 'max' (inclusive).
static inline int clamp(int value, int min, int max) { return (value < min ? min : (value > max ? max : value)); }

static inline int abs(int value) { return (value >= 0 ? value : -value); }

static inline int signum(int value) { return (value > 0 ? 1 : (value < 0 ? -1 : 0) ); }

static inline float clampf(float value, float min, float max) { return (value < min ? min : (value > max ? max : value)); }

// Set the position of an object to specified x and y coordinates
static inline void set_object_position(volatile object_attributes *object, int x, int y) {
	object->attribute_zero = (object->attribute_zero & ~OBJECT_ATTRIBUTE_ZERO_Y_MASK) | (y & OBJECT_ATTRIBUTE_ZERO_Y_MASK);
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_X_MASK) | (x & OBJECT_ATTRIBUTE_ONE_X_MASK);
}

static inline void set_object_horizontal_flip(volatile object_attributes *object, int isFlip){
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK) | (isFlip ? OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK : 0);
}

static inline void set_object_vertical_flip(volatile object_attributes* object, int isFlip){
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_VERTICAL_FLIP_MASK) | (isFlip ? OBJECT_ATTRIBUTE_ONE_VERTICAL_FLIP_MASK : 0);
}

// Sound synth
#define SNDWV_SQR1    0x01
#define SNDWV_SQR2    0x02
#define SNDWV_WAVE    0x04
#define SNDWV_NOISE   0x08

#define REG_SND_CH1_SWP  (*(volatile uint16*)0x4000060)  
#define REG_SND_CH1_LDE  (*(volatile uint16*)0x4000062)
#define REG_SND_CH1_FRQ  (*(volatile uint32*)0x4000064)

#define REG_SND_DMGCNT  (*(volatile uint16*)0x4000080)
#define REG_SND_DSCNT   (*(volatile uint16*)0x4000082)
#define REG_SND_STAT    (*(volatile uint16*)0x4000084)    
	
#define SFREQ_RESET 0x8000

#define SND_RATE(freq, oct) ( 2048-(freq>>(4+(oct))) )
	
#define SDMG_BUILD(_lmode, _rmode, _lvol, _rvol)    \
    ( ((_lvol)&7) | (((_rvol)&7)<<4) | ((_lmode)<<8) | ((_rmode)<<12) )
	
#define SSQR_ENV_BUILD(ivol, dir, time)				\
	(  ((ivol)<<12) | ((dir)<<11) | (((time)&7)<<8) )
