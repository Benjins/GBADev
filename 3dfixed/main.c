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

static inline int min(int a, int b){return a < b ? a : b;}
static inline int max(int a, int b){return a > b ? a : b;}
static inline int clamp(int val, int _min, int _max){ return min(_max, max(val, _min));}

static inline int abs(int x){return (x < 0) ? -x : x;}

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

static inline fixed mySin(fixed in){
	
	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return in - fixPow(in,3)/6 + fixPow(in,5)/120 - fixPow(in,7)/5040 + fixPow(in,9)/362880;
}

static inline fixed myCos(fixed in){

	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return FIXED_ONE - fixMult(in,in)/4 + fixPow(in,4)/24 - fixPow(in,6)/720 + fixPow(in,8)/40320;
}

static inline fixed mySqrt(fixed in){
	int reduce = (in >= makeFixed(4));
	if(reduce){
		in /= 4;
	}
	
	in -= FIXED_ONE;
	
	fixed guess = FIXED_ONE + in/2 - fixMult(in,in)/8 + fixPow(in,3)/16 - 5*fixPow(in,4)/128 + 7*fixPow(in,5)/256;
	
	in += FIXED_ONE;
	
	for(int i = 0; i < 10; i++){
		if(guess == 0){
			break;
		}
		guess = (guess + fixDiv(in, guess))/2;
	}
	
	if(reduce){
		guess *= 2;
	}
	
	return abs(guess);
}

static inline int roundFixedToInt(fixed x){
	return (x + FIXED_ONE/2) >> FIXED_DECIMAL;
}

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
	
	Wall w0 = {{fixedFromFlt(-2.2f), fixedFromFlt(1.2f)}, {fixedFromFlt(2.8f), fixedFromFlt(20.0f)}, RGB15(20,20,20)};
	AddWall(w0);
	
	Wall w1 = {{fixedFromFlt(-1.0f), fixedFromFlt(2.0f)}, {fixedFromFlt(1.0f), fixedFromFlt(2.0f)}, RGB15(20,8,8)};
	AddWall(w1);
	
	Wall w2 = {{fixedFromFlt(-1.4f), fixedFromFlt(1.9f)}, {fixedFromFlt(1.5f), fixedFromFlt(2.9f)}, RGB15(4,25,15)};
	AddWall(w2);
	
	/*
	Wall w3 = {{-0.2f, 0.5f}, {0.2f, 0.5f}, RGB15(15,15,2)};
	AddWall(w3);
	
	Wall w4 = {{-2.0f, 3.5f}, {2.5f, 0.9f}, RGB15(3,6,25)};
	AddWall(w4);
	*/
	
	/*
	Wall w0 = {{fixedFromFlt(-10.0f), fixedFromFlt(-10.0f)}, {fixedFromFlt(-10.0f), fixedFromFlt(10.0f)}, RGB15(30,2,2)};
	AddWall(w0);
	
	Wall w1 = {{fixedFromFlt(-10.0f), fixedFromFlt(-10.0f)}, {fixedFromFlt(10.0f), fixedFromFlt(-10.0f)}, RGB15(2,30,2)};
	AddWall(w1);
	
	Wall w2 = {{fixedFromFlt(-10.0f), fixedFromFlt(10.0f)}, {fixedFromFlt(10.0f), fixedFromFlt(10.0f)}, RGB15(2,2,30)};
	AddWall(w2);
	
	Wall w3 = {{fixedFromFlt(10.0f), fixedFromFlt(10.0f)}, {fixedFromFlt(10.0f), fixedFromFlt(-10.0f)}, RGB15(28,28,4)};
	AddWall(w3);
	*/
	
	fixed cameraPos[2] = {makeFixed(-2), 0};
	fixed cameraRot = 0.0f;
	
	uint32 prevKeys = 0;
	
	fixed turnSpeed = fixedFromFlt(4.0f);
	int isFast = 1;
	
	rgb15 cols[SCREEN_WIDTH];
	uint8 heights[SCREEN_WIDTH];
	
	while(1){
		uint32 keyStates = ~REG_KEY_INPUT & KEY_ANY;
		
		fixed forwardVec[2] = {mySin(cameraRot), myCos(cameraRot)};
		fixed rightVec[2]   = {forwardVec[1], -forwardVec[0]};
		
		if (keyStates & KEY_RIGHT) { cameraRot += turnSpeed;  if(cameraRot < -makeFixed(180)) {cameraRot += makeFixed(360);}}
		if (keyStates & KEY_LEFT) { cameraRot -= turnSpeed; if(cameraRot >  makeFixed(180)) {cameraRot -= makeFixed(360);}}
		
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
		
		if (keyStates & BUTTON_A) { 
			for(int i = 0; i < 2; i++){
				cameraPos[i] -= rightVec[i]/4;
			} 
		}
		
		if (keyStates & BUTTON_B) {
			for(int i = 0; i < 2; i++){
				cameraPos[i] += rightVec[i]/4;
			}
		}
		
		if ((keyStates & BUTTON_SELECT) && !(prevKeys & BUTTON_SELECT)) {
			isFast = !isFast;
			
			if(isFast){
				turnSpeed = fixedFromFlt(4.0f);
			}
			else{
				turnSpeed = fixedFromFlt(0.5f);
			}
		}
		
		if ((keyStates & BUTTON_START) && !(prevKeys & BUTTON_START)) {
			cameraRot = 0;
		}
		
		static const fixed camWidth = fixedFromFlt(1.0f);
		fixed rayOrigin[2] = {cameraPos[0] - fixMult(rightVec[0],camWidth/2), 
							  cameraPos[1] - fixMult(rightVec[1],camWidth/2)};
		
		rgb15 empty = RGB15(3,3,3);
		rgb15 floorCol = RGB15(8,8,8);
		rgb15 ceilCol = RGB15(16,16,16);
		
		for(int x = 0; x < SCREEN_WIDTH; x ++){
			RaycastHit hit = Raycast(rayOrigin, forwardVec);
			
			if(hit.wasHit){
				fixed hitHeight = fixDiv(makeFixed(SCREEN_HEIGHT), hit.depth + fixedFromFlt(0.5f))*100;
				int hitScreenHeight = roundFixedToInt(hitHeight);
				
				cols[x] = hit.col;
				heights[x] = hitScreenHeight;
			}
			else{
				heights[x] = 0;
			}
			
			for(int i = 0; i < 2; i++){
				rayOrigin[i] += fixMult(rightVec[0],camWidth)/SCREEN_WIDTH;
			}
		}
		
		VBlankIntrWait();
		
		for(int x = 0; x < SCREEN_WIDTH; x++){
			int start = clamp(SCREEN_HEIGHT/2 - (heights[x]/2), 0, SCREEN_HEIGHT);
			int end   = clamp(SCREEN_HEIGHT/2 + (heights[x]/2), 0, SCREEN_HEIGHT);
			
			for(int y = 0; y < start; y++){
				FRAME_MEM[y*SCREEN_WIDTH+x] = ceilCol;
			}
			for(int y = start; y < end; y++){
				FRAME_MEM[y*SCREEN_WIDTH+x] = cols[x];
			}
			for(int y = end; y < SCREEN_HEIGHT; y++){
				FRAME_MEM[y*SCREEN_WIDTH+x] = floorCol;
			}
		}
		
		prevKeys = keyStates;
	}
	
	return 0;
}

RaycastHit Raycast(fixed* cameraPos, fixed* cameraDir){
	RaycastHit hit;
	hit.depth = fixedFromFlt(1000000.0f);
	hit.wasHit = 0;
	
	for(int i = 0; i < wallCount; i++){
		fixed originToStart[2] = {walls[i].startPos[0] - cameraPos[0], walls[i].startPos[1] - cameraPos[1]};
		fixed originToEnd[2]   = {walls[i].endPos[0] - cameraPos[0],   walls[i].endPos[1] - cameraPos[1]};
		
		fixed startCrossDir = fixMult(originToStart[0],cameraDir[1]) - fixMult(originToStart[1],cameraDir[0]);
		fixed endCrossDir   = fixMult(originToEnd[0],cameraDir[1])   - fixMult(originToEnd[1],cameraDir[0]);
		
		fixed dirDotStart = fixMult(cameraDir[0], originToStart[0]) + fixMult(cameraDir[1], originToStart[1]);
		fixed dirDotEnd = fixMult(cameraDir[0], originToEnd[0]) + fixMult(cameraDir[1], originToEnd[1]);
		
		if(endCrossDir*startCrossDir < 0 && (dirDotStart > 0 || dirDotEnd > 0)){
			fixed startDist = fixMult(originToStart[0],originToStart[0]) + fixMult(originToStart[1],originToStart[1]);
			fixed endDist = fixMult(originToEnd[0],originToEnd[0]) + fixMult(originToEnd[1],originToEnd[1]);
			
			//startDist = mySqrt(startDist);
			//endDist = mySqrt(endDist);
			
			fixed projSpan = abs(endCrossDir) + abs(startCrossDir);
			fixed startPortion = fixDiv(abs(startCrossDir), projSpan);
			fixed castDist = fixMult(startPortion, startDist) + fixMult(FIXED_ONE - startPortion, endDist);
			
			hit.wasHit = 1;
			
			if(hit.depth > castDist){
				hit.depth = castDist;
				hit.col = walls[i].col;
			}
		}
	}
	
	return hit;
}