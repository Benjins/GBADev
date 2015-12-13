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

typedef struct{
	float startPos[2];
	float endPos[2];
	rgb15 col;
} Wall;

typedef struct{
	float depth;
	rgb15 col;
	int wasHit;
} RaycastHit;

#define MAX_WALL_COUNT 50

Wall walls[MAX_WALL_COUNT] = {};
int wallCount = 0;

RaycastHit Raycast(float* cameraPos, float* cameraDir);

void AddWall(Wall wall){
	walls[wallCount] = wall;
	
	wallCount++;
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
	
	Wall w1 = {{-1.0f, 2.0f}, {1.0f, 2.0f}, RGB15(20,8,8)};
	AddWall(w1);
	
	Wall w2 = {{-1.4f, 1.5f}, {0.5f, 2.9f}, RGB15(4,25,15)};
	AddWall(w2);
	
	Wall w3 = {{-0.2f, 0.5f}, {0.2f, 0.5f}, RGB15(15,15,15)};
	AddWall(w3);
	
	Wall w4 = {{-2.4f, 3.5f}, {2.5f, 0.9f}, RGB15(3,6,25)};
	AddWall(w4);
	
	rgb15 col = RGB15(20,20,10);
	
	float cameraPos[2] = {};
	float cameraRot = 0.0f;
	
	uint32 prevKeys = 0;
	
	while(1){
		uint32 keyStates = ~REG_KEY_INPUT & KEY_ANY;
		
		VBlankIntrWait();
		
		//Might not need it if already drawing to each pixel
#if 0
		for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
			FRAME_MEM[i] = 0;
		}
#endif
		
		float forwardVec[2] = {mySin(cameraRot), myCos(cameraRot)};
		float rightVec[2]   = {forwardVec[1], -forwardVec[0]};
		
		static const float turnSpeed = 10.0f;
		
		if (keyStates & KEY_RIGHT) { cameraRot -= turnSpeed;  if(cameraRot < -180.0f) {cameraRot += 360.0f;}}
		if (keyStates & KEY_LEFT) { cameraRot += turnSpeed; if(cameraRot >  180.0f) {cameraRot -= 360.0f;}}
		
		if (keyStates & KEY_UP) { 
			for(int i = 0; i < 2; i++){
				cameraPos[i] += forwardVec[i];
			} 
		}
		
		if (keyStates & KEY_DOWN) {
			for(int i = 0; i < 2; i++){
				cameraPos[i] -= forwardVec[i];
			}
		}
		
		static const float camWidth = 0.3f;
		float rayOrigin[2] = {cameraPos[0] - rightVec[0]*camWidth/2, cameraPos[1] - rightVec[1]*camWidth/2};
		
		rgb15 empty = RGB15(3,3,3);
		
		for(int x = 0; x < SCREEN_WIDTH; x += 3){
			
			RaycastHit hit = Raycast(rayOrigin, forwardVec);
			
			if(hit.wasHit){
				int hitScreenHeight = (int)(SCREEN_HEIGHT / hit.depth / 2 + 0.5f);
				
				int start = clamp(SCREEN_HEIGHT/2 - hitScreenHeight/2, 0, SCREEN_HEIGHT);
				int end   = clamp(SCREEN_HEIGHT/2 + hitScreenHeight/2, 0, SCREEN_HEIGHT);
				
				for(int y = 0; y < start; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = empty;
					FRAME_MEM[y*SCREEN_WIDTH+x+1] = empty;
					FRAME_MEM[y*SCREEN_WIDTH+x+2] = empty;
				}
				for(int y = start; y < end; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = hit.col;
					FRAME_MEM[y*SCREEN_WIDTH+x+1] = hit.col;
					FRAME_MEM[y*SCREEN_WIDTH+x+2] = hit.col;
				}
				for(int y = end; y < SCREEN_HEIGHT; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = empty;
					FRAME_MEM[y*SCREEN_WIDTH+x+1] = empty;
					FRAME_MEM[y*SCREEN_WIDTH+x+2] = empty;
				}
			}
			else{
				for(int y = 0; y < SCREEN_HEIGHT; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = empty;
					FRAME_MEM[y*SCREEN_WIDTH+x+1] = empty;
					FRAME_MEM[y*SCREEN_WIDTH+x+2] = empty;
				}
			}
			
			for(int i = 0; i < 2; i++){
				rayOrigin[i] += rightVec[0]*camWidth/SCREEN_WIDTH*3;
			}
		}
		
		prevKeys = keyStates;
	}
	
	return 0;
}

RaycastHit Raycast(float* cameraPos, float* cameraDir){
	RaycastHit hit;
	hit.depth = 1000.0f;
	hit.wasHit = 0;
	
	for(int i = 0; i < wallCount; i++){
		float wallVec[2] = {walls[i].endPos[0] - walls[i].startPos[0], walls[i].endPos[1] - walls[i].startPos[1]};
		float diffVec[2] = {cameraPos[0] - walls[i].startPos[0], cameraPos[1] - walls[i].startPos[1]};
		
		float projMultNum = 0.0f;
		float projMultDen = 0.0f;
		for(int k = 0; k < 2; k++){
			projMultNum += diffVec[k]*wallVec[k];
			projMultDen += wallVec[k]*wallVec[k];
		}
		
		float projMult = projMultNum/projMultDen;
		
		float projVec[2] = {wallVec[0]*projMult, wallVec[1]*projMult};
		float projVecSub[2] = {projVec[0] - diffVec[0], projVec[1] - diffVec[1]};
		
		float castDist = (cameraDir[0]*projVecSub[0]+cameraDir[1]*projVecSub[1])/(projVecSub[0]*projVecSub[0]+projVecSub[1]*projVecSub[1]);
		float wallDist = projMultDen;
		
		if(castDist > 0 && castDist < wallDist && hit.depth*hit.depth > castDist){
			hit.wasHit = 1;
			hit.depth = mySqrt(castDist);
			hit.col = walls[i].col;
		}
	}
	
	return hit;
}