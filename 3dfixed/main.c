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



#define FIXED_DECIMAL 8
#define FIXED_ONE (1 << FIXED_DECIMAL)
typedef int32 fixed;

static inline fixed makeFixed(int x){
	return x << FIXED_DECIMAL;
}

#define fixedFromFlt(x) ((fixed)((x) * FIXED_ONE))

static const fixed deg2rad = fixedFromFlt(3.1415926536f/180);

static inline fixed fixMult(fixed a, fixed b){
	return (a * b) >> FIXED_DECIMAL;
}

static inline fixed fixDiv(fixed a, fixed b){
	return ((a/b) << FIXED_DECIMAL) + ((a%b) << FIXED_DECIMAL) / b;
}

static inline fixed fixPow(fixed b, int e){
	fixed answer = FIXED_ONE;
	for(int i = 0; i < e; i++){
		answer = fixMult(answer, b);
	}
	
	return answer;
}

static fixed mySin(fixed in){
	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return in - fixPow(in,3)/6 + fixPow(in,5)/120 - fixPow(in,7)/5040;
}

static fixed myCos(fixed in){
	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return 1.0f - fixMult(in,in)/4 + fixPow(in,4)/24 - fixPow(in,6)/720;
}

static fixed mySqrt(fixed in){
	in -= FIXED_ONE;
	
	return 1 + in/2 - fixMult(in,in)/8 + fixPow(in,3)/16 - 5*fixPow(in,4)/128 + 7*fixPow(in,5)/256;
}

static inline int roundFixedToInt(fixed x){
	return (x + FIXED_ONE/2) >> FIXED_DECIMAL;
}

static inline int min(int a, int b){return a < b ? a : b;}
static inline int max(int a, int b){return a > b ? a : b;}
static inline int clamp(int val, int _min, int _max){ return min(_max, max(val, _min));}

typedef struct{
	fixed startPos[2];
	fixed endPos[2];
	rgb15 col;
} Wall;

typedef struct{
	fixed depth;
	rgb15 col;
	int wasHit;
} RaycastHit;

#define MAX_WALL_COUNT 50

Wall walls[MAX_WALL_COUNT] = {};
int wallCount = 0;

RaycastHit Raycast(fixed* cameraPos, fixed* cameraDir);

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
	
	Wall w1 = {{fixedFromFlt(-1.0f), fixedFromFlt(2.0f)}, {fixedFromFlt(1.0f), fixedFromFlt(2.0f)}, RGB15(20,8,8)};
	AddWall(w1);
	
	Wall w2 = {{fixedFromFlt(-1.4f), fixedFromFlt(1.5f)}, {fixedFromFlt(0.5f), fixedFromFlt(2.9f)}, RGB15(4,25,15)};
	AddWall(w2);
	
	/*
	Wall w3 = {{-0.2f, 0.5f}, {0.2f, 0.5f}, RGB15(15,15,15)};
	AddWall(w3);
	
	Wall w4 = {{-2.4f, 3.5f}, {2.5f, 0.9f}, RGB15(3,6,25)};
	AddWall(w4);
	*/
	
	rgb15 col = RGB15(20,20,10);
	
	fixed cameraPos[2] = {};
	fixed cameraRot = 0.0f;
	
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
		
		fixed forwardVec[2] = {mySin(cameraRot), myCos(cameraRot)};
		fixed rightVec[2]   = {forwardVec[1], -forwardVec[0]};
		
		static const fixed turnSpeed = fixedFromFlt(6.0f);
		
		if (keyStates & KEY_RIGHT) { cameraRot -= turnSpeed;  if(cameraRot < -makeFixed(180)) {cameraRot += makeFixed(360);}}
		if (keyStates & KEY_LEFT) { cameraRot += turnSpeed; if(cameraRot >  makeFixed(180)) {cameraRot -= makeFixed(360);}}
		
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
		
		static const fixed camWidth = fixedFromFlt(0.3f);
		fixed rayOrigin[2] = {cameraPos[0] - fixMult(rightVec[0],camWidth/2), 
							  cameraPos[1] - fixMult(rightVec[1],camWidth/2)};
		
		rgb15 empty = RGB15(3,3,3);
		rgb15 floorCol = RGB15(8,8,8);
		rgb15 ceilCol = RGB15(16,16,16);
		
		for(int x = 0; x < SCREEN_WIDTH; x ++){
			
			RaycastHit hit = Raycast(rayOrigin, forwardVec);
			
			if(hit.wasHit){
				fixed hitHeight = fixDiv(makeFixed(SCREEN_HEIGHT), hit.depth)/2;
				int hitScreenHeight = roundFixedToInt(hitHeight);
				
				int start = clamp(SCREEN_HEIGHT/2 + hitScreenHeight/2, 0, SCREEN_HEIGHT);
				int end   = clamp(SCREEN_HEIGHT/2 - hitScreenHeight/2, 0, SCREEN_HEIGHT);
				
				for(int y = 0; y < start; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = ceilCol;
				}
				for(int y = start; y < end; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = hit.col;
				}
				for(int y = end; y < SCREEN_HEIGHT; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = floorCol;
				}
			}
			else{
				for(int y = 0; y < SCREEN_HEIGHT/2; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = ceilCol;
				}
				for(int y = SCREEN_HEIGHT/2; y < SCREEN_HEIGHT; y++){
					FRAME_MEM[y*SCREEN_WIDTH+x] = floorCol;
				}
			}
			
			for(int i = 0; i < 2; i++){
				rayOrigin[i] += fixMult(rightVec[0],camWidth)/SCREEN_WIDTH;
			}
		}
		
		prevKeys = keyStates;
	}
	
	return 0;
}

RaycastHit Raycast(fixed* cameraPos, fixed* cameraDir){
	RaycastHit hit;
	hit.depth = fixedFromFlt(1000.0f);
	hit.wasHit = 0;
	
	for(int i = 0; i < wallCount; i++){
		fixed wallVec[2] = {walls[i].endPos[0] - walls[i].startPos[0], walls[i].endPos[1] - walls[i].startPos[1]};
		fixed diffVec[2] = {cameraPos[0] - walls[i].startPos[0], cameraPos[1] - walls[i].startPos[1]};
		
		fixed projMultNum = 0;
		fixed projMultDen = 0;
		for(int k = 0; k < 2; k++){
			projMultNum += fixMult(diffVec[k],wallVec[k]);
			projMultDen += fixMult(wallVec[k],wallVec[k]);
		}
		
		fixed projMult = fixDiv(projMultNum,projMultDen);
		
		fixed projVec[2] = {fixMult(wallVec[0],projMult), fixMult(wallVec[1],projMult)};
		fixed projVecSub[2] = {projVec[0] - diffVec[0], projVec[1] - diffVec[1]};
		
		fixed castNum = fixMult(cameraDir[0], projVecSub[0])+fixMult(cameraDir[1], projVecSub[1]);
		fixed castDen = fixMult(projVecSub[0],projVecSub[0])+fixMult(projVecSub[1],projVecSub[1]);
		fixed castDist = fixDiv(castNum, castDen);
						
		fixed wallDist = projMultDen;
		
		fixed sqrtCastDist = mySqrt(castDist);
		if(castDist > 0 && castDist < wallDist && hit.depth > sqrtCastDist){
			hit.wasHit = 1;
			hit.depth = sqrtCastDist;
			hit.col = walls[i].col;
		}
	}
	
	return hit;
}