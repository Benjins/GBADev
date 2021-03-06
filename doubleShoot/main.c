#include "../common/gba.h"

#include "../common/fixed.c"

#include "assets.h"

static inline void set_sprite_memory(Sprite sprite, volatile uint16* memory){
	int memSize = (sprite.height * sprite.width) / 4;
	for(int i = 0; i < memSize; i++){
		uint16 mem = 0;
		for(int pix = 0; pix < 4; pix++){
			uint16 pixelCol = sprite.data[i*4+pix];
			mem |= (pixelCol << (pix*4));
		}
		memory[i] = mem;
	}
}

#define MAX_WHITE_ENEMY_COUNT 24
#define MAX_GREY_ENEMY_COUNT 24
#define MAX_WHITE_BULLET_COUNT 32
#define MAX_GREY_BULLET_COUNT 32

volatile object_attributes* playerAttribs = &oam_memory[0];
volatile object_attributes* whiteEnemyAttribs = &oam_memory[1];
volatile object_attributes* greyEnemyAttribs = &oam_memory[MAX_WHITE_ENEMY_COUNT+1];
volatile object_attributes* whiteBulletAttribs = &oam_memory[MAX_GREY_ENEMY_COUNT+MAX_WHITE_ENEMY_COUNT+1];
volatile object_attributes* greyBulletAttribs = &oam_memory[MAX_WHITE_BULLET_COUNT+MAX_GREY_ENEMY_COUNT+MAX_WHITE_ENEMY_COUNT+1];

enum Direction{
	D_Up = 0,
	D_Down = 1,
	D_Left = 2,
	D_Right = 3,
	D_UL = 4,
	D_UR = 5,
	D_DL = 6,
	D_DR = 7,
	D_Count
};

fixed directionVectors[D_Count][2] = {
	{0, -FIXED_ONE},
	{0,  FIXED_ONE},
	{-FIXED_ONE, 0},
	{ FIXED_ONE, 0},
	{-fixedFromFlt(0.707f), -fixedFromFlt(0.707f)},
	{ fixedFromFlt(0.707f), -fixedFromFlt(0.707f)},
	{-fixedFromFlt(0.707f),  fixedFromFlt(0.707f)},
	{ fixedFromFlt(0.707f),  fixedFromFlt(0.707f)}
};

typedef struct{
	fixed pos[2];
	Direction dir;
} Entity;

int whiteEnemyCount = 0;
Entity whiteEnemies[MAX_WHITE_ENEMY_COUNT];
int greyEnemyCount = 0;
Entity greyEnemies[MAX_GREY_ENEMY_COUNT];

const int enemySpriteSize = 24;

int whiteBulletCount = 0;
Entity whiteBullets[MAX_WHITE_BULLET_COUNT];
int greyBulletCount = 0;
Entity greyBullets[MAX_GREY_BULLET_COUNT];

const int bulletSpriteSize = 8;

Entity playerEntity;

const int playerSpriteSize = 32;

#define ADD_ENTITY(ents, entCount) &((ents)[(entCount)++])

static inline void RemoveEntity(Entity* ents, int idx, int* entCount){
	ents[idx] = ents[*entCount - 1];
	(*entCount)--;
}

static inline void SetObjectAttribs(volatile object_attributes* attribs, Entity* ents, int entCount, int maxCount, int spriteSize){
	for (int i = 0; i < entCount; i++){
		int x = roundFixedToInt(ents[i].pos[0]) - spriteSize/2;
		int y = roundFixedToInt(ents[i].pos[1]) - spriteSize/2;
		set_object_position(&attribs[i], x, y);
	}

	for (int i = entCount; i < maxCount; i++){
		set_object_position(&attribs[i], -33, -33);
	}
}

static inline void UpdateEntityPositionsWrap(Entity* ents, int entCount, fixed speed){
	for (int i = 0; i < entCount; i++){
		fixed* moveVec = directionVectors[ents[i].dir];
		ents[i].pos[0] += fixMult(moveVec[0], speed);
		ents[i].pos[1] += fixMult(moveVec[1], speed);

		ents[i].pos[0] = (ents[i].pos[0] + makeFixed(255)) % makeFixed(255);
		ents[i].pos[1] = (ents[i].pos[1] + makeFixed(255)) % makeFixed(255);
	}
}

static inline void UpdateEntityPositionsKillOOB(Entity* ents, int* entCount, fixed speed){
	for (int i = 0; i < *entCount; i++){
		fixed* moveVec = directionVectors[ents[i].dir];
		ents[i].pos[0] += fixMult(moveVec[0], speed);
		ents[i].pos[1] += fixMult(moveVec[1], speed);

		if (ents[i].pos[0] > makeFixed(SCREEN_WIDTH)  || ents[i].pos[0] < 0
		 || ents[i].pos[1] > makeFixed(SCREEN_HEIGHT) || ents[i].pos[1] < 0){
			 RemoveEntity(ents, i, entCount);
			 i--;
		 }
	}
}

#define FIXED_ABS_DIFF(a, b) (((a) > (b)) ? (a) - (b) : (b) - (a))

static_assert(FIXED_ABS_DIFF(12, 15) == 3, "FIXED_ABS_DIFF works");
static_assert(FIXED_ABS_DIFF(15, 12) == 3, "FIXED_ABS_DIFF works");

static inline void KillEntitiesIfCollisions(Entity* ents1, int* entityCount1, int entitySize1, Entity* ents2, int* entityCount2, int entitySize2){
	const fixed entRadiiTotal = makeFixed(entitySize1/2) + makeFixed(entitySize2/2);

	for (int i = 0; i < *entityCount1; i++){
		for (int j = 0; j < *entityCount2; j++){
			fixed xDiff = FIXED_ABS_DIFF(ents1[i].pos[0], ents2[j].pos[0]);
			fixed yDiff = FIXED_ABS_DIFF(ents1[i].pos[1], ents2[j].pos[1]);

			if (xDiff < entRadiiTotal && yDiff < entRadiiTotal){
				RemoveEntity(ents1, i, entityCount1);
				RemoveEntity(ents2, j, entityCount2);
				i--;
				break;
			}
		}
	}
}

static inline void SplitEnemiesAndKillBulletsIfCollide( Entity* enemyEnts, int* enemyEntCount, int enemySize,
														Entity* bulletEnts, int* bulletEntCount, int bulletSize){
	const fixed entRadiiTotal = makeFixed(enemySize/2) + makeFixed(bulletSize/2);

	for (int i = 0; i < *enemyEntCount; i++){
		for (int j = 0; j < *bulletEntCount; j++){
			fixed xDiff = FIXED_ABS_DIFF(enemyEnts[i].pos[0], bulletEnts[j].pos[0]);
			fixed yDiff = FIXED_ABS_DIFF(enemyEnts[i].pos[1], bulletEnts[j].pos[1]);

			if (xDiff < entRadiiTotal && yDiff < entRadiiTotal){
				if (enemyEnts[i].dir == D_Right || enemyEnts[i].dir == D_Left){
					Entity* newEnt1 = ADD_ENTITY(enemyEnts, *enemyEntCount);

					newEnt1->pos[0] = enemyEnts[i].pos[0];
					newEnt1->pos[1] = enemyEnts[i].pos[1];
					newEnt1->dir = D_Up;

					enemyEnts[i].dir = D_Down;
				}
				else{
					Entity* newEnt1 = ADD_ENTITY(enemyEnts, *enemyEntCount);

					newEnt1->pos[0] = enemyEnts[i].pos[0];
					newEnt1->pos[1] = enemyEnts[i].pos[1];
					newEnt1->dir = D_Left;

					enemyEnts[i].dir = D_Right;
				}

				RemoveEntity(bulletEnts, j, bulletEntCount);
				break;
			}
		}
	}
}

static inline int CheckIfCollisions(Entity* ents1, int ent1Count, int ent1Size, Entity* ents2, int ent2Count, int ent2Size){
	const fixed entRadiiTotal = makeFixed(ent1Size/2) + makeFixed(ent2Size/2);

	for (int i = 0; i < ent1Count; i++){
		for (int j = 0; j < ent2Count; j++){
			fixed xDiff = FIXED_ABS_DIFF(ents1[i].pos[0], ents2[j].pos[0]);
			fixed yDiff = FIXED_ABS_DIFF(ents1[i].pos[1], ents2[j].pos[1]);

			if (xDiff < entRadiiTotal && yDiff < entRadiiTotal){
				return 1;
			}
		}
	}


	return 0;
}

int main(void) {
	INT_VECTOR = InterruptMain;
	
	BNS_REG_IME	= 0;
	REG_DISPSTAT |= LCDC_VBL;
	BNS_REG_IE |= IRQ_VBLANK;
	BNS_REG_IME	= 1;

	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		object_palette_memory[i] = paletteColors[i];
		bg0_palette_memory[i] = paletteColors[i];
	}

	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }

	// Set the display parameters to enable objects, and use a 1D object->tile mapping, and enable BG2
	REG_DISPLAY = 0x1000 | 0x0040;

	{
		volatile uint16* whiteEnemyTileMem = (uint16 *)tile_memory[4][1];
		set_sprite_memory(whiteEnemySprite, whiteEnemyTileMem);

		volatile uint16* greyEnemyTileMem = (uint16 *)tile_memory[4][17];
		set_sprite_memory(greyEnemySprite, greyEnemyTileMem);

		volatile uint16* playerTileMem = (uint16 *)tile_memory[4][33];
		set_sprite_memory(playerSprite, playerTileMem);

		volatile uint16* greyBulletTileMem = (uint16 *)tile_memory[4][49];
		set_sprite_memory(greyBulletSprite, greyBulletTileMem);

		volatile uint16* whiteBulletTileMem = (uint16 *)tile_memory[4][50];
		set_sprite_memory(whiteBulletSprite, whiteBulletTileMem);

		volatile uint16* playerHorizTileMem = (uint16 *)tile_memory[4][51];
		set_sprite_memory(playerSpriteHoriz, playerHorizTileMem);

		volatile uint16* playerDiagTileMem = (uint16 *)tile_memory[4][67];
		set_sprite_memory(playerSpriteDiag, playerDiagTileMem);
	}

	for (int i = 0; i < 128; i++){
		oam_memory[i].attribute_zero = 0;
		oam_memory[i].attribute_one = 0;
		oam_memory[i].attribute_two = 0;
	}

	playerAttribs->attribute_zero = 0;
	playerAttribs->attribute_one = 0x8000;
	playerAttribs->attribute_two = 33;

	for (int i = 0; i < MAX_WHITE_ENEMY_COUNT; i++){
		whiteEnemyAttribs[i].attribute_zero = 0;
		whiteEnemyAttribs[i].attribute_one = 0x8000;
		whiteEnemyAttribs[i].attribute_two = 1;
		set_object_position(&whiteEnemyAttribs[i], -33, -33);
	}

	for (int i = 0; i < MAX_GREY_ENEMY_COUNT; i++){
		greyEnemyAttribs[i].attribute_zero = 0;
		greyEnemyAttribs[i].attribute_one = 0x8000;
		greyEnemyAttribs[i].attribute_two = 17;
		set_object_position(&greyEnemyAttribs[i], -33, -33);
	}

	for (int i = 0; i < MAX_WHITE_BULLET_COUNT; i++){
		whiteBulletAttribs[i].attribute_zero = 0;
		whiteBulletAttribs[i].attribute_one = 0;
		whiteBulletAttribs[i].attribute_two = 50;
		set_object_position(&whiteBulletAttribs[i], -10, -10);
	}

	for (int i = 0; i < MAX_GREY_BULLET_COUNT; i++){
		greyBulletAttribs[i].attribute_zero = 0;
		greyBulletAttribs[i].attribute_one = 0;
		greyBulletAttribs[i].attribute_two = 49;
		set_object_position(&greyBulletAttribs[i], -10, -10);
	}

	uint32 prevKeys = 0;

	RESTART:

	Entity* whiteEnemy = ADD_ENTITY(whiteEnemies, whiteEnemyCount);
	whiteEnemy->pos[0] = makeFixed(50);
	whiteEnemy->pos[1] = makeFixed(50);
	whiteEnemy->dir = D_Down;

	Entity* greyEnemy = ADD_ENTITY(greyEnemies, greyEnemyCount);
	greyEnemy->pos[0] = makeFixed(120);
	greyEnemy->pos[1] = makeFixed(30);
	greyEnemy->dir = D_Left;

	playerEntity.pos[0] = makeFixed(80);
	playerEntity.pos[1] = makeFixed(110);

	while (1) {
		asm("swi 0x05" ::: "r0", "r1", "r2", "r3");

		uint32 key_states = ~REG_KEY_INPUT & KEY_ANY;

		Direction newPlayerDir = D_Count;
		if (key_states & KEY_UP){
			newPlayerDir = D_Up;
		}

		if (key_states & KEY_DOWN){
			newPlayerDir = D_Down;
		}

		if (key_states & KEY_LEFT){
			if (newPlayerDir == D_Count){
				newPlayerDir = D_Left;
			}
			else if (newPlayerDir == D_Up){
				newPlayerDir = D_UL;
			}
			else if (newPlayerDir == D_Down){
				newPlayerDir = D_DL;
			}
		}

		if (key_states & KEY_RIGHT){
			if (newPlayerDir == D_Count){
				newPlayerDir = D_Right;
			}
			else if (newPlayerDir == D_Up){
				newPlayerDir = D_UR;
			}
			else if (newPlayerDir == D_Down){
				newPlayerDir = D_DR;
			}
		}
		
		if (newPlayerDir != D_Count){
			playerEntity.dir = newPlayerDir;
			playerEntity.pos[0] += directionVectors[newPlayerDir][0];
			playerEntity.pos[1] += directionVectors[newPlayerDir][1];
		}

		switch (playerEntity.dir){
			case D_Up: {
				set_object_horizontal_flip(playerAttribs, false);
				set_object_vertical_flip(playerAttribs, false);
				playerAttribs->attribute_two = 33;
			} break;

			case D_Down: {
				set_object_horizontal_flip(playerAttribs, false);
				set_object_vertical_flip(playerAttribs, true);
				playerAttribs->attribute_two = 33;
			} break;

			case D_Left: {
				set_object_horizontal_flip(playerAttribs, true);
				set_object_vertical_flip(playerAttribs, false);
				playerAttribs->attribute_two = 51;
			} break;

			case D_Right: {
				set_object_horizontal_flip(playerAttribs, false);
				set_object_vertical_flip(playerAttribs, false);
				playerAttribs->attribute_two = 51;
			} break;

			case D_UL: {
				set_object_horizontal_flip(playerAttribs, true);
				set_object_vertical_flip(playerAttribs, false);
				playerAttribs->attribute_two = 67;
			} break;

			case D_UR: {
				set_object_horizontal_flip(playerAttribs, false);
				set_object_vertical_flip(playerAttribs, false);
				playerAttribs->attribute_two = 67;
			} break;

			case D_DL: {
				set_object_horizontal_flip(playerAttribs, true);
				set_object_vertical_flip(playerAttribs, true);
				playerAttribs->attribute_two = 67;
			} break;

			case D_DR: {
				set_object_horizontal_flip(playerAttribs, false);
				set_object_vertical_flip(playerAttribs, true);
				playerAttribs->attribute_two = 67;
			} break;

			default: {
				// Uh....
			} break;
		}

		if (!(key_states & BUTTON_A) && (prevKeys & BUTTON_A)){
			Entity* greyBullet = ADD_ENTITY(greyBullets, greyBulletCount);
			greyBullet->pos[0] = playerEntity.pos[0] + 10 * directionVectors[playerEntity.dir][0];
			greyBullet->pos[1] = playerEntity.pos[1] + 10 * directionVectors[playerEntity.dir][1];
			greyBullet->dir = playerEntity.dir;
		}

		if (!(key_states & BUTTON_B) && (prevKeys & BUTTON_B)){
			Entity* whiteBullet = ADD_ENTITY(whiteBullets, whiteBulletCount);
			whiteBullet->pos[0] = playerEntity.pos[0] + 10 * directionVectors[playerEntity.dir][0];
			whiteBullet->pos[1] = playerEntity.pos[1] + 10 * directionVectors[playerEntity.dir][1];
			whiteBullet->dir = playerEntity.dir;
		}

		UpdateEntityPositionsWrap(whiteEnemies, whiteEnemyCount, fixedFromFlt(0.8f));
		UpdateEntityPositionsWrap(greyEnemies, greyEnemyCount, fixedFromFlt(0.8f));

		UpdateEntityPositionsKillOOB(whiteBullets, &whiteBulletCount, fixedFromFlt(2.2f));
		UpdateEntityPositionsKillOOB(greyBullets, &greyBulletCount, fixedFromFlt(2.2f));

		KillEntitiesIfCollisions(whiteEnemies, &whiteEnemyCount, enemySpriteSize, whiteBullets, &whiteBulletCount, bulletSpriteSize);
		KillEntitiesIfCollisions(greyEnemies,  &greyEnemyCount,  enemySpriteSize, greyBullets,  &greyBulletCount,  bulletSpriteSize);

		SplitEnemiesAndKillBulletsIfCollide(whiteEnemies, &whiteEnemyCount, enemySpriteSize, greyBullets,  &greyBulletCount,  bulletSpriteSize);
		SplitEnemiesAndKillBulletsIfCollide(greyEnemies,  &greyEnemyCount,  enemySpriteSize, whiteBullets, &whiteBulletCount, bulletSpriteSize);

		if (CheckIfCollisions(whiteEnemies, whiteEnemyCount, enemySpriteSize, &playerEntity, 1, playerSpriteSize)
		 || CheckIfCollisions(greyEnemies, greyEnemyCount, enemySpriteSize, &playerEntity, 1, playerSpriteSize)
		|| ((prevKeys & BUTTON_SELECT) && !(key_states & BUTTON_SELECT))){
			whiteEnemyCount = 0;
			greyEnemyCount = 0;
			whiteBulletCount = 0;
			greyBulletCount = 0;
			prevKeys = key_states;
			goto RESTART;
		}

		SetObjectAttribs(playerAttribs, &playerEntity, 1, 1, playerSpriteSize);
		SetObjectAttribs(whiteEnemyAttribs, whiteEnemies, whiteEnemyCount, MAX_WHITE_ENEMY_COUNT, enemySpriteSize);
		SetObjectAttribs(greyEnemyAttribs, greyEnemies, greyEnemyCount, MAX_GREY_ENEMY_COUNT, enemySpriteSize);
		SetObjectAttribs(whiteBulletAttribs, whiteBullets, whiteBulletCount, MAX_WHITE_BULLET_COUNT, bulletSpriteSize);
		SetObjectAttribs(greyBulletAttribs, greyBullets, greyBulletCount, MAX_GREY_BULLET_COUNT, bulletSpriteSize);

		prevKeys = key_states;
	}

	return 0;
}