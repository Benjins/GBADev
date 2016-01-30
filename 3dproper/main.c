#include <gba_interrupt.h>

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
#define BUTTON_R 0x0100
#define BUTTON_L 0x0200

static inline int clamp(int value, int min, int max) { return (value < min ? min : (value > max ? max : value)); }

static inline int abs(int value) { return (value >= 0 ? value : -value); }

#include "fixed.h"

typedef struct{
	union{
		struct{
			fixed x;
			fixed y;
		};
		fixed vec[2];
	};
} Vec2;

fixed DotProduct(Vec2 a, Vec2 b){
	return fixMult(a.x, b.x) + fixMult(a.y, b.y);
}

Vec2 VecSub(Vec2 a, Vec2 b){
	Vec2 retVal = {a.x - b.x, a.y - b.y};
	return retVal;
}

Vec2 VecAdd(Vec2 a, Vec2 b){
	Vec2 retVal = {a.x + b.x, a.y + b.y};
	return retVal;
}

Vec2 VecScale(Vec2 v, fixed s){
	Vec2 retVal = {fixMult(v.x, s), fixMult(v.y, s)};
	return retVal;
}

Vec2 AngleToVec(fixed angle){
	Vec2 forward = {mySin(angle), myCos(angle)};
	return forward;
}

typedef struct{
	Vec2 start;
	Vec2 end;
	rgb15 col;
} Wall;

#define MAX_WALL_COUNT 20
Wall walls[MAX_WALL_COUNT];
int wallCount = 0;

void AddWall(Wall wall){
	walls[wallCount] = wall;
	wallCount++;
}

int main(){
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	REG_DISPLAY = 0x0403;

	for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
		FRAME_MEM[i] = 0;
	}

	Wall firstWall = {{fixedFromFlt(-5.0f), fixedFromFlt(0.0f)}, {fixedFromFlt(5.0f), fixedFromFlt(4.0f)}, 0x3448};
	AddWall(firstWall);
	
	fixed playerAngle = 0;
	Vec2 playerPos = {fixedFromFlt(0.0f), fixedFromFlt(-4.0f)};
	
	uint32 keyStates = 0;
	uint32 prevStates = 0;
	
	while(1){
		asm("swi 0x05");
		
		/*
		for(int i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++){
			FRAME_MEM[i] = 0;
		}
		*/
		
		keyStates = ~REG_KEY_INPUT & KEY_ANY;
		
		Vec2 playerForward = AngleToVec(playerAngle);
		Vec2 playerRight = {playerForward.y, -playerForward.x};
		
		if(keyStates & KEY_UP){
			playerPos = VecAdd(playerPos, VecScale(playerForward, fixedFromFlt(0.f)));
		}
		if(keyStates & KEY_DOWN){
			playerPos = VecSub(playerPos, VecScale(playerForward, fixedFromFlt(0.5f)));
		}
		if(keyStates & KEY_LEFT){
			playerPos = VecSub(playerPos, VecScale(playerRight, fixedFromFlt(0.f)));
		}
		if(keyStates & KEY_RIGHT){
			playerPos = VecAdd(playerPos, VecScale(playerRight, fixedFromFlt(0.f)));
		}
		
		if(keyStates & BUTTON_L){
			playerAngle += fixedFromFlt(2.5f);
		}
		if(keyStates & BUTTON_R){
			playerAngle -= fixedFromFlt(2.5f);
		}
		
		//uint16 
		
		for(int i = 0; i < wallCount; i++){
			Vec2 playerToWallStart = VecSub(walls[i].start, playerPos);
			Vec2 playerToWallEnd   = VecSub(walls[i].end,   playerPos);
			
			fixed forwardDotToStart = DotProduct(playerToWallStart, playerForward);
			fixed forwardDotToEnd   = DotProduct(playerToWallEnd,   playerForward);
			
			if(forwardDotToStart > 0 || forwardDotToEnd > 0){
				Vec2 startProj = VecSub(walls[i].start, VecScale(playerForward, forwardDotToStart));
				Vec2 endProj   = VecSub(walls[i].end,   VecScale(playerForward, forwardDotToEnd));
				
				fixed startProjDotRight = DotProduct(startProj, playerRight);
				fixed   endProjDotRight = DotProduct(endProj,   playerRight);
				
				int32 pixelStart = roundFixedToInt(startProjDotRight*SCREEN_WIDTH)+SCREEN_WIDTH/2;
				int32 pixelEnd   = roundFixedToInt(  endProjDotRight*SCREEN_WIDTH)+SCREEN_WIDTH/2;
				
				fixed startDepth = mySqrt(forwardDotToStart);
				fixed endDepth = mySqrt(forwardDotToEnd);
				
				if(pixelStart > pixelEnd){
					int32 temp = pixelStart;
					pixelStart = pixelEnd;
					pixelEnd = temp;
					
					fixed depthTmp = startDepth;
					startDepth = endDepth;
					endDepth = depthTmp;
				}
				
				if(pixelEnd < 0 || pixelStart >= SCREEN_WIDTH){
					continue;
				}
				else{
					if(pixelStart < 0){
						fixed ratio = makeFixed(-pixelStart)/makeFixed(pixelEnd-pixelStart);
						pixelStart = 0;
						startDepth = fixMult(FIXED_ONE-ratio, startDepth) + fixMult(ratio, endDepth);
					}
					
					if(pixelEnd >= SCREEN_WIDTH){
						fixed ratio = makeFixed(pixelEnd - SCREEN_WIDTH)/makeFixed(pixelEnd);
						pixelEnd = SCREEN_WIDTH - 1;
						endDepth = fixMult(FIXED_ONE-ratio, endDepth) + fixMult(ratio, startDepth);
					}
					
					fixed depthIncrement = fixDiv(endDepth - startDepth, makeFixed(pixelEnd - pixelStart + 1));
					fixed currDepth = startDepth;
					rgb15 wallCol = walls[i].col;
					for(int32 x = pixelStart; x <= pixelEnd; x++){
						int32 wallHeight = roundFixedToInt(fixDiv(makeFixed(SCREEN_HEIGHT), currDepth));
						
						int32 y = 0;
						for(; y < SCREEN_HEIGHT/2-wallHeight; y++){
							FRAME_MEM[y*SCREEN_WIDTH+x] = 0x4433;
						}
						for(;y < SCREEN_HEIGHT/2+wallHeight; y++){
							FRAME_MEM[y*SCREEN_WIDTH+x] = wallCol;
						}
						for(;y < SCREEN_HEIGHT; y++){
							FRAME_MEM[y*SCREEN_WIDTH+x] = 0x2211;
						}
						
						currDepth += depthIncrement;
					}
					
				}
			}
		}
		
		prevStates = keyStates;
	}

	return 0;
}