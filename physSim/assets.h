typedef struct{int palIdx; int width; int height; unsigned short* data;} Sprite;
 typedef struct{Sprite map; Sprite* bgSprites; int* spriteFlags; int bgCount;} Background;
static unsigned short ballSprite_data[] = {
0,0,0,0,0,1,1,1,0,0,0,0,1,2,2,2,0,0,0,1,2,2,3,3,0,0,1,2,3,3,3,3,0,1,2,3,3,3,3,3,1,2,2,3,3,6,2,2,1,2,3,3,2,2,6,2,1,2,3,3,3,2,2,2,1,1,0,0,0,0,0,0,2,2,2,1,0,0,0,0,3,3,2,2,1,0,0,0,3,3,3,3,2,1,0,0,3,3,3,3,3,2,1,0,3,3,3,3,3,3,2,1,2,3,3,3,3,3,2,1,6,6,3,3,3,3,2,1,1,2,3,3,4,5,5,2,1,2,3,3,4,5,5,4,0,1,2,3,4,4,4,4,0,1,1,2,3,3,3,3,0,0,1,1,2,2,3,3,0,0,0,1,1,2,2,2,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,2,2,3,3,3,3,2,1,4,3,3,3,3,3,2,1,4,4,3,3,3,2,1,0,3,3,3,3,2,2,1,0,3,3,2,2,2,1,0,0,2,2,2,1,1,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,};
static Sprite ballSprite = {0, 16, 16, ballSprite_data
};

rgb15 paletteColors[] = {
	0,310,4607,1855,12153,3990,4221,0,0,0,0,0,0,0,0,0,};
