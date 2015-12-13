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

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#define MEM_IO   0x04000000
#define MEM_PAL  0x05000000
#define MEM_VRAM 0x06000000
#define MEM_OAM  0x07000000

#define FRAME_MEM ((volatile uint16*)MEM_VRAM)

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

// Form a 16-bit BGR GBA colour from three component values
static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

static const float deg2rad = 3.14159265358979323846f/180;

static float mySin(float in){
	in += 180.0f;
	int fmod = (int)(in/360.0f);
	in -= 360.0f*fmod;
	in -= 180.0f;
	
	in = in * deg2rad;
	return in - (in*in*in)/6 + (in*in*in*in*in)/120 - (in*in*in*in*in*in*in)/5040;
}

static float myCos(float in){
	in += 180.0f;
	int fmod = (int)(in/360.0f);
	in -= 360.0f*fmod;
	in -= 180.0f;
	
	in = in * deg2rad;
	return 1.0f - (in*in)/4 + (in*in*in*in)/24 - (in*in*in*in*in*in)/720;
}

static float mySqrt(float in){
	in -= 1.0f;
	
	return 1 + in/2 - (in*in)/8 + (in*in*in)/16 - (in*in*in*in)*5.0f/128 + (in*in*in*in*in)*7.0f/256;
}

static inline int min(int a, int b){return a < b ? a : b;}
static inline int max(int a, int b){return a > b ? a : b;}
static inline int clamp(int val, int _min, int _max){ return min(_max, max(val, _min));}

static inline int abs(int x){return x >= 0 ? x : -x;}
static inline float absf(float x){return x >= 0 ? x : -x;}


//TODO: Clip lines to screen
void DrawLine(int _x0, int _y0, int _x1, int _y1, rgb15 col, int thickness){
	int x0 = _x0;
	int x1 = _x1;
	int y0 = _y0;
	int y1 = _y1;
	
	if(_x0 > _x1){
		x0 = _x1;
		x1 = _x0;
		y0 = _y1;
		y1 = _y0;
	}

	
	if(x0 == x1){
		y0 = min(_y0, _y1);
		y1 = max(_y0, _y1);
		
		for(int y = y0; y <= y1; y++){
			FRAME_MEM[y*SCREEN_WIDTH+x0] = col;
		}
	}
	else{
		int dx = x1 - x0;
		int dy = abs(y1 - y0);
		int  D = 0;//2*dy - dx;
		
		FRAME_MEM[y0*SCREEN_WIDTH+x0] = col;
		int y = y0;
		for(int x = x0; x < x1; x++){
			FRAME_MEM[y*SCREEN_WIDTH+x] = col;
			D = D + (2*dy);
			while(D > 0){
				y = y + (y1 > y0 ? 1 : -1);
				FRAME_MEM[y*SCREEN_WIDTH+x] = col;
				D = D - (2*dx);
			}
		}
	}
}

void DrawTriangle(int* v0, int* v1, int* v2, rgb15 col){
	DrawLine(v0[0], v0[1], v1[0], v1[1], col, 1.0f);
	DrawLine(v1[0], v1[1], v2[0], v2[1], col, 1.0f);
	DrawLine(v2[0], v2[1], v0[0], v0[1], col, 1.0f);
}

int main(void) {
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	//REG_DISPLAY = 0x0403;
	
	unsigned char ioramData[2] = {0x03, 0x04};
	
	volatile int16* ioram = (int16*)0x04000000;
	*ioram = *((int16*)ioramData);
	
	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
		FRAME_MEM[i] = 0;
	}
	
	uint32 prevKeys = 0;
	
	int cursorPos[6] = {0 , 0, SCREEN_WIDTH/2, SCREEN_HEIGHT/2, SCREEN_WIDTH, 50};
	int vertIdx = 0;
	
	while(1){
		uint32 keyStates = ~REG_KEY_INPUT & KEY_ANY;
		
		VBlankIntrWait();
		
		//Might not need it if already drawing to each pixel
#if 0
		for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
			FRAME_MEM[i] = 0;
		}
#endif

		DrawTriangle(cursorPos, &cursorPos[2], &cursorPos[4], 0);
		
		if (keyStates & KEY_RIGHT) {cursorPos[vertIdx*2]++;}
		if (keyStates & KEY_LEFT) {cursorPos[vertIdx*2]--;}
		
		if (keyStates & KEY_UP) {cursorPos[vertIdx*2+1]--;}		
		if (keyStates & KEY_DOWN) {cursorPos[vertIdx*2+1]++;}
		
		
		if ((keyStates & BUTTON_SELECT) && !(prevKeys & BUTTON_SELECT)) { vertIdx = (vertIdx + 1) % 3;}
		
		rgb15 col = RGB15(30,30,30);
		
		//DrawLine(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, cursorPos[0], cursorPos[1], col, 1.0f);
		
		DrawTriangle(cursorPos, &cursorPos[2], &cursorPos[4], col);
		
		rgb15 red   = RGB15(30,4,4);
		rgb15 green = RGB15(4,30,4);
		
		//DrawLine(0,   0, 50, 50, col, 1.0f);
		//DrawLine(50,  0, 50, 50, col, 1.0f);
		//DrawLine(101, 0, 100, 50, red, 1.0f);
		//DrawLine(99,  0, 100, 50, green, 1.0f);
		
		prevKeys = keyStates;
	}
	
	return 0;
}
