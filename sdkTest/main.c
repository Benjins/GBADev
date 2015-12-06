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
typedef struct object_attributes {
	uint16 attribute_zero;
	uint16 attribute_one;
	uint16 attribute_two;
	uint16 pad;
} __attribute__((aligned(4))) object_attributes;
typedef uint32 tile4bpp[8];
typedef tile4bpp tile_block[512];

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 160

#define MEM_IO   0x04000000
#define MEM_PAL  0x05000000
#define MEM_VRAM 0x06000000
#define MEM_OAM  0x07000000

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
	
#define OBJECT_ATTRIBUTE_ZERO_Y_MASK  0xFF
#define OBJECT_ATTRIBUTE_ONE_X_MASK  0x1FF
#define OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK  0x1000

#define oam_memory ((volatile object_attributes *)MEM_OAM)
#define tile_memory ((volatile tile_block *)MEM_VRAM)
#define object_palette_memory ((volatile rgb15 *)(MEM_PAL + 0x200))

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

static inline void  enable_sound(){REG_SND_STAT |= 0x80;}
static inline void disable_sound(){REG_SND_STAT &= ~0x80;}

// Form a 16-bit BGR GBA colour from three component values
static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

// Set the position of an object to specified x and y coordinates
static inline void set_object_position(volatile object_attributes *object, int x, int y) {
	object->attribute_zero = (object->attribute_zero & ~OBJECT_ATTRIBUTE_ZERO_Y_MASK) | (y & OBJECT_ATTRIBUTE_ZERO_Y_MASK);
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_X_MASK) | (x & OBJECT_ATTRIBUTE_ONE_X_MASK);
}

static inline void set_object_horizontal_flip(volatile object_attributes *object, int isFlip){
	object->attribute_one = (object->attribute_one & ~OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK) | (isFlip ? OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK : 0);
}

// Clamp 'value' in the range 'min' to 'max' (inclusive).
static inline int clamp(int value, int min, int max) { return (value < min ? min : (value > max ? max : value)); }

static inline int abs(int value) { return (value >= 0 ? value : -value); }

static inline float clampf(float value, float min, float max) { return (value < min ? min : (value > max ? max : value)); }

#include "assets.h"

Sprite* goalUISprites[] = {&goalUI1Sprite, &goalUI2Sprite, &goalUI3Sprite, &goalUI4Sprite};

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

#define MAX_PLATFORM_COUNT 10
#define MAX_BULLET_COUNT 44
#define MAX_ENEMY_COUNT 8
#define MAX_LETTER_COUNT 42
#define MAX_UI_COUNT 15

volatile object_attributes* uiTextAttribs = &oam_memory[0];
volatile object_attributes* arrowAttribs = &oam_memory[MAX_LETTER_COUNT];
volatile object_attributes* uiTileAttribs = &oam_memory[MAX_LETTER_COUNT+1];
volatile object_attributes* playerAttr = &oam_memory[MAX_UI_COUNT+MAX_LETTER_COUNT+1];
volatile object_attributes* platformAttribs = &oam_memory[MAX_UI_COUNT+MAX_LETTER_COUNT+2];
volatile object_attributes* enemyAttribs = &oam_memory[MAX_UI_COUNT+MAX_LETTER_COUNT+MAX_PLATFORM_COUNT + 2];
volatile object_attributes* bulletAttribs = &oam_memory[MAX_UI_COUNT+MAX_LETTER_COUNT+MAX_PLATFORM_COUNT + MAX_ENEMY_COUNT + 2];
volatile object_attributes* goalAttribs = &oam_memory[MAX_UI_COUNT+MAX_LETTER_COUNT+MAX_PLATFORM_COUNT + MAX_ENEMY_COUNT + MAX_BULLET_COUNT + 2];
volatile object_attributes* goalUiAttribs = &oam_memory[MAX_UI_COUNT+MAX_LETTER_COUNT+MAX_PLATFORM_COUNT + MAX_ENEMY_COUNT + MAX_BULLET_COUNT + 3];

#define ENEMY_GOING_LEFT 0x0010
#define ENEMY_PLATFORM_MASK 0x000F
#define ENEMY_SHOOT_TIMER_MASK 0xFF00

#define BULLET_GOING_LEFT 0x01
#define BULLET_IS_PLAYER 0x02

typedef struct platform{
	int16 position[2];
} platform;

typedef struct enemy{
	int16 position[2];
	int16 flags;
} enemy;

typedef struct bullet{
	int16 position[2];
	int16 flags;
} bullet;

typedef struct{
	int* platInit;
	int* enemyInit;
	
	int platCount;
	int enemyCount;
	
	int goalPos[2];
	int reachedGoal;
}LevelInit;

bullet bullets[MAX_BULLET_COUNT];
platform platforms[MAX_PLATFORM_COUNT];
enemy enemies[MAX_ENEMY_COUNT];

int bulletCount = 0;
int platformCount = 0;
int enemyCount = 0;
int letterCount = 0;

#define MAX_TEXT_STRING_COUNT 16

int textIndexCount = 0;
int textIndices[MAX_TEXT_STRING_COUNT] = {};

bullet* AddBullet(){
	bullet* newBullet = &bullets[bulletCount];
	bulletCount++;
	return newBullet;
}

platform* AddPlatform(){
	platform* newPlatform = &platforms[platformCount];
	platformCount++;
	return newPlatform;
}

enemy* AddEnemy(){
	enemy* newEnemy = &enemies[enemyCount];
	enemyCount++;
	return newEnemy;
}

void RemoveBullet(bullet* bul){
	bulletCount--;
	set_object_position(&bulletAttribs[bulletCount], -10, -10);
	*bul = bullets[bulletCount];
}

void RemoveEnemy(enemy* enem){
	enemyCount--;
	set_object_position(&enemyAttribs[enemyCount], -10, -10);
	*enem = enemies[enemyCount];
}

void ClearText(){
	for(int i = 0; i < MAX_LETTER_COUNT; i++){
		uiTextAttribs[i].attribute_two = 0;
	}
}

void PopText(){
	if(textIndexCount > 0){
		textIndexCount--;
		letterCount = textIndices[textIndexCount];
		
		for(int i = letterCount; i < MAX_LETTER_COUNT; i++){
			uiTextAttribs[i].attribute_two = 0;
		}
	}
}

void PushText(char* text, int x, int y){
	static const int caseMask = 'a' ^ 'A';
	
	textIndices[textIndexCount] = letterCount;
	textIndexCount++;
	
	int index = letterCount;
	int position=0;
	for(char* cursor = text; *cursor != '\0'; cursor++){
		if(*cursor == ' '){
			position++;
			continue;
		}
		
		char baseLetter = *cursor | caseMask;
		if(baseLetter >= 'a' && baseLetter <= 'z'){
			int spriteIdx = 14 + baseLetter - 'a';
			uiTextAttribs[index].attribute_two = spriteIdx;
			set_object_position(&uiTextAttribs[index], 8*position+x, y);
			index++;
			position++;
		}
	}
	
	letterCount = index;
}

int platInitLvl1[][2] = {{20,100}, {40,130}, {70,70}, {70,122}, {120,52}, {86,130}, {10,22}};
int enemyInitLvl1[][2] = {{40,122}, {70,70}, {16, 20}};

int platInitLvl2[][2] = {{70,70}, {70,122}, {160, 30}, {100, 85}, {180, 70}, {120,52}, {86,130}};
int enemyInitLvl2[][2] = {{70,70}, {160, 30}, {120, 52}};

int platInitLvl3[][2] = {{40,120}, {60, 105}, {55, 85}, {150, 40}, {130, 15}};
int enemyInitLvl3[][2] = {{40,120}, {60, 105}, {55, 85}};

int platInitLvl4[][2] = {{45,135}, {40,120}, {60, 105}, {55, 85}, {75, 68}, {55, 45}};
int enemyInitLvl4[][2] = {{40,120}, {60, 105}, {55, 85}};

#define MAKE_LEVEL(n, g1, g2) {&platInitLvl##n[0][0], &enemyInitLvl##n[0][0], ARRAY_LENGTH(platInitLvl##n), ARRAY_LENGTH(enemyInitLvl##n), {g1, g2}, 0}

LevelInit levels[] = {
					MAKE_LEVEL(1, 10, 15)
				   ,MAKE_LEVEL(2, 180, 63)
				   ,MAKE_LEVEL(3, 150, 33)
				   ,MAKE_LEVEL(4, 55, 38)};

#undef MAKE_LEVEL 
				   
int levelIdx = 0;	

int goalsReached = 0;			   

void LoadLevel(LevelInit init){
	platformCount = 0;
	bulletCount = 0;
	enemyCount = 0;

	for(int i = 0; i < init.platCount; i++){
		platform* newPlat = AddPlatform();
		newPlat->position[0] = init.platInit[2*i];
		newPlat->position[1] = init.platInit[2*i+1];
	}
	
	for(int i = 0; i < init.enemyCount; i++){
		enemy* newEnemy = AddEnemy();
		newEnemy->position[0] = init.enemyInit[2*i];
		newEnemy->position[1] = init.enemyInit[2*i+1];
	}
	
	for(int i = 0; i < MAX_PLATFORM_COUNT; i++){
		volatile object_attributes* platformAttrib = &platformAttribs[i];
		platformAttrib->attribute_zero = 0x4000;
		platformAttrib->attribute_one = 0x4000; 
		platformAttrib->attribute_two = 1; 
	
		if(i >= platformCount){
			set_object_position(platformAttrib, -10, -10);
		}
	}
	
	for(int i = 0; i < MAX_ENEMY_COUNT; i++){
		volatile object_attributes* enemyAttrib = &enemyAttribs[i];
		enemyAttrib->attribute_zero = 0; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
		enemyAttrib->attribute_one = 0; // This sprite has a size of 8x8 when the SQUARE shape is set.
		enemyAttrib->attribute_two = 6;
		
		if(i >= enemyCount){
			set_object_position(enemyAttrib, -10, -10);
		}
	}
	
	for(int i = 0; i < MAX_BULLET_COUNT; i++){
		volatile object_attributes* bulletAttrib = &bulletAttribs[i];
		bulletAttrib->attribute_zero = 0; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
		bulletAttrib->attribute_one = 0; // This sprite has a size of 8x8 when the SQUARE shape is set.
		bulletAttrib->attribute_two = 7;
		set_object_position(bulletAttrib, -10, -10);
	}
	
	for(int i = 0; i < platformCount; i++){
		set_object_position(&platformAttribs[i], platforms[i].position[0], platforms[i].position[1]);
	}
	
	for(int i = 0; i < enemyCount; i++){
		int closestPlatIdx = 0;
		int closestPlatDist = 10000;
		for(int platIdx = 0; platIdx < platformCount; platIdx++){
			int xDiff = (enemies[i].position[0] - platforms[platIdx].position[0]);
			int yDiff = (enemies[i].position[1] - platforms[platIdx].position[1]);
			int dist = xDiff * xDiff + yDiff * yDiff;
			
			if(dist < closestPlatDist){
				closestPlatIdx = platIdx;
				closestPlatDist = dist;
			}
			
		}
		
		enemies[i].flags = enemies[i].flags & ~ENEMY_PLATFORM_MASK;
		enemies[i].flags = enemies[i].flags | (ENEMY_PLATFORM_MASK & closestPlatIdx);
		
		set_object_position(&enemyAttribs[i], enemies[i].position[0], enemies[i].position[1]);
	}
	
	for(int i = 0; i < bulletCount; i++){
		set_object_position(&bulletAttribs[i], bullets[i].position[0], bullets[i].position[1]);
	}
	
	char lvlText[16] = "level _";
	
	lvlText[6] = ("abcd"[levelIdx]);
	
	PopText();
	PushText(lvlText, 8, 150);
}

typedef enum{
	PLAY,
	MENU
} GameMode;

GameMode currMode = PLAY;

typedef enum {
	RESUME,
	QUIT,
	RESET,
	OPTIONS
} MenuAction;

typedef struct{
	char* label;
	MenuAction action;
} MenuItem ;

MenuItem menuItems[] = {{"resume", RESUME}, {"reset", RESET}};

void Reset(){
	goalsReached = 0;
	
	volatile uint16* ui_goal_memory = (uint16 *)tile_memory[4][9];
	set_sprite_memory(goalUISprite, ui_goal_memory);
	
	levelIdx = 0;
	LoadLevel(levels[0]);
	
	for(int i = 0; i < ARRAY_LENGTH(levels); i++){
		levels[i].reachedGoal = 0;
	}
}

void EnterMenuMode(){
	for(int i = 0; i < ARRAY_LENGTH(menuItems); i++){
		PushText(menuItems[i].label, 180, 20+i*10);
	}
	
	currMode = MENU;
}

void ExitMenuMode(){
	for(int i = 0; i < ARRAY_LENGTH(menuItems); i++){
		PopText();
	}
	
	set_object_position(arrowAttribs, -10, -10);
	
	currMode = PLAY;
}

void ExecuteMenuAction(MenuAction action){
	if(action == RESUME){
		ExitMenuMode();
	}
	else if(action == RESET){
		ExitMenuMode();
		Reset();
	}
}

int main(void) {
	// Write the tiles for our sprites into the 4th tile block in VRAM.
	// Particularly, four tiles for an 8x32 paddle sprite, and 1 tile for an 8x8 ball sprite.
	// 0x1111 = 0001000100010001 [4bpp = colour index 1, colour index 1, colour index 1, colour index 1]
	// 0x2222 = 0002000200020002 [4bpp = colour index 2, colour index 2, colour index 2, colour index 2]
	// NOTE: We're using our own memory writing code here to avoid the byte-granular writes that
	// something like 'memset' might make (GBA VRAM doesn't support byte-granular writes).
	
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }
	
	volatile uint16* paddle_tile_memory = (uint16 *)tile_memory[4][1];
	set_sprite_memory(platformSprite, paddle_tile_memory);
	//for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { paddle_tile_memory[i] = 0x1111; }
	
	volatile uint16* ball_player_memory = (uint16 *)tile_memory[4][5];
	set_sprite_memory(ballSprite, ball_player_memory);
	//for (int i = 0; i < (sizeof(tile4bpp) / 2); ++i) { ball_player_memory[i] = 0x2222; }
	
	volatile uint16* enemy_memory = (uint16 *)tile_memory[4][6];
	set_sprite_memory(enemySprite, enemy_memory);
	//for (int i = 0; i < (sizeof(tile4bpp) / 2); ++i) { enemy_memory[i] = 0x3333; }
	
	volatile uint16* bullet_memory = (uint16 *)tile_memory[4][7];
	set_sprite_memory(bulletSprite, bullet_memory);
	//for (int i = 0; i < (sizeof(tile4bpp) / 2); ++i) { bullet_memory[i] = 0x4554; }
	
	volatile uint16* goal_memory = (uint16 *)tile_memory[4][8];
	set_sprite_memory(goalSprite, goal_memory);
	
	volatile uint16* ui_goal_memory = (uint16 *)tile_memory[4][9];
	set_sprite_memory(goalUISprite, ui_goal_memory);
	
	Sprite font[] = {aFont, bFont, cFont, dFont, eFont, fFont, gFont, hFont, iFont,
					jFont, kFont, lFont, mFont, nFont, oFont, pFont, qFont, rFont, sFont,
					tFont, uFont, vFont, wFont, xFont, yFont, zFont};
	
	volatile uint16* arrowMemory = (uint16 *)tile_memory[4][13];
	set_sprite_memory(arrowSprite, arrowMemory);
	
	for(int i = 0; i < ARRAY_LENGTH(font); i++){
		volatile uint16* uiFontMemory = (uint16 *)tile_memory[4][14+i];
		set_sprite_memory(font[i], uiFontMemory);
	}
	
	// Write the colour palette for our sprites into the first palette of
	// 16 colours in colour palette memory (this palette has index 0).
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		object_palette_memory[i] = paletteColors[i];
	}
	
	playerAttr->attribute_zero = 0; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
	playerAttr->attribute_one = 0; // This sprite has a size of 8x8 when the SQUARE shape is set.
	playerAttr->attribute_two = 5; // This sprite's base tile is the fifth tile in tile block 4, and this sprite should use colour palette 0.

	goalAttribs->attribute_zero = 0; 
	goalAttribs->attribute_one = 0;  
	goalAttribs->attribute_two = 8;  
	
	
	arrowAttribs->attribute_zero = 0; 
	arrowAttribs->attribute_one = 0;  
	arrowAttribs->attribute_two = 13;
	set_object_position(arrowAttribs, -10, -10);
	
	goalUiAttribs->attribute_zero = 0x4000; 
	goalUiAttribs->attribute_one = 0x4000;  
	goalUiAttribs->attribute_two = 9; 

	set_object_position(goalUiAttribs, SCREEN_WIDTH - 32, 0);
	
	const int player_width = 8, player_height = 8, ball_width = 8, ball_height = 8, platform_width = 32, platform_height = 8;
	const float playerSpeed = 1.8f;
	float player_x = 5.0f, player_y = 96.0f;
	float playerVel = 0;
	int isGrounded = 0;
	int isFlip=0;
	int prevKeys = 0;
	
	set_object_position(playerAttr, (int)(player_x+0.5f), (int)(player_y+0.5f));
	
	levelIdx = 0;
	LoadLevel(levels[levelIdx]);
	
	// Set the display parameters to enable objects, and use a 1D object->tile mapping.
	REG_DISPLAY = 0x1000 | 0x0040;
	
	//Turn on sound
	enable_sound();
	
	//Set left and right to full volume square waves
	REG_SND_DMGCNT = SDMG_BUILD(SNDWV_SQR1, SNDWV_SQR1, 7, 7);
	
	//Set DMG volume to 100%
	REG_SND_DSCNT = 0x02;
	
	//Set square generator for channel 1 to 100% volume, and sustain
	REG_SND_CH1_LDE = SSQR_ENV_BUILD(12, 0, 2) | 0x0080;
	
	//Set frequency of channel 1
	REG_SND_CH1_FRQ = 0x40;
	
	REG_SND_CH1_SWP = 0x0008;
	
	// Our main game loop
	uint32 key_states = 0;
	
	int menuIdx = 0;
	
	while (1) {
		VBlankIntrWait();
		
		// Get current key states (REG_KEY_INPUT stores the states inverted)
		key_states = ~REG_KEY_INPUT & KEY_ANY;
		
		if(currMode == PLAY){
			// Note that our physics update is tied to the framerate rather than a fixed timestep.
			int player_max_clamp_y = SCREEN_HEIGHT - player_height;
			int player_max_clamp_x = SCREEN_WIDTH - player_width;
			
			float new_player_x = player_x;
			
			if ((key_states & BUTTON_A) && isGrounded) { playerVel = 130.0f;}
			if (key_states & KEY_LEFT) { new_player_x = clampf(player_x - playerSpeed, 0, player_max_clamp_x);isFlip=1;}
			if (key_states & KEY_RIGHT) { new_player_x = clampf(player_x + playerSpeed, 0, player_max_clamp_x);isFlip=0;}
			
			/*
			//Useful for debugging text push and pop
			if((key_states & KEY_UP) && !(prevKeys & KEY_UP)){
				PushText("qwerty", 50, 100);
			}
			
			if((key_states & KEY_DOWN) && !(prevKeys & KEY_DOWN)){
				PopText();
			}
			*/
			
			if((key_states & BUTTON_B) && !(prevKeys & BUTTON_B)){
				bullet* newBullet = AddBullet();
				newBullet->position[0] = player_x;
				newBullet->position[1] = player_y;
				newBullet->flags = BULLET_IS_PLAYER | (isFlip ? BULLET_GOING_LEFT : 0);
			}
			
			if((key_states & BUTTON_START) && !(prevKeys & BUTTON_START)){
				EnterMenuMode();
			}
			
			//platforms[2].position[1] = (platforms[2].position[1] - 1) % 200;
			
			if(!isGrounded || playerVel >= 0){
				player_y = clampf(player_y - playerVel/60, 0, player_max_clamp_y);
			}
			
			isGrounded = player_y >= player_max_clamp_y;
			if(!isGrounded && playerVel <= 0){
				for(int i = 0; i < platformCount; i++){
					short yDiff = player_y - platforms[i].position[1];
					int isOnPlatform = (yDiff > -player_height && yDiff < 0);
					short difference = (int)(player_x + 0.5f) - platforms[i].position[0];
					isOnPlatform &= (difference > -player_width && difference < platform_width);
					isGrounded |= isOnPlatform;
					if(isOnPlatform){
						player_y = platforms[i].position[1] - player_height;
						break;
					}
				}
			}
			
			if(new_player_x != player_x){
				int hitsWall = 0;
				for(int i = 0; i < platformCount; i++){
					short yDiff = player_y - platforms[i].position[1];
					int isOnPlatform = (yDiff > -player_height && yDiff < player_height);
					
					float difference = (new_player_x - platforms[i].position[0]);
					float oldDifference = (player_x - platforms[i].position[0]);
					isOnPlatform &= (difference >= -player_width && difference <= platform_width);
					isOnPlatform &= (oldDifference <= -player_width || oldDifference >= platform_width);
					
					hitsWall |= isOnPlatform;
					if(hitsWall){
						//player_x = platforms[i].position[0] + clampf(difference, 0, platform_width);
						break;
					}
				}
				
				if(!hitsWall){
					player_x = new_player_x;
				}
			}
			
			if(isGrounded){
				playerVel = 0.0f;
			}
			else{
				playerVel -= 0.017f * 250;
			}
			
			set_object_position(playerAttr, player_x, player_y);
			set_object_horizontal_flip(playerAttr, isFlip);
			
			for(int i = 0; i < platformCount; i++){
				set_object_position(&platformAttribs[i], platforms[i].position[0], platforms[i].position[1]);
			}
			
			for(int i = 0; i < enemyCount; i++){
				enemies[i].position[1] = platforms[enemies[i].flags & ENEMY_PLATFORM_MASK].position[1] - platform_height;
				if((enemies[i].flags & ENEMY_GOING_LEFT)){
					enemies[i].position[0]--;
					if(enemies[i].position[0] < (platforms[enemies[i].flags & ENEMY_PLATFORM_MASK].position[0])){
						enemies[i].flags = enemies[i].flags & ~ENEMY_GOING_LEFT;
					}
				}
				else{
					enemies[i].position[0]++;
					if(enemies[i].position[0] >= (platforms[enemies[i].flags & ENEMY_PLATFORM_MASK].position[0] + platform_width)){
						enemies[i].flags = enemies[i].flags | ENEMY_GOING_LEFT;
					}
				}
				
				short newTimer = ((enemies[i].flags & ENEMY_SHOOT_TIMER_MASK) >> 8) + 1;
				enemies[i].flags = (enemies[i].flags & ~ENEMY_SHOOT_TIMER_MASK) | (newTimer << 8);
				
				if(newTimer % 96 == 0){
					bullet* newBullet = AddBullet();
					for(int p = 0; p < 2; p++){
						newBullet->position[p] = enemies[i].position[p];
					}
					newBullet->flags = ((enemies[i].flags & ENEMY_GOING_LEFT) ? BULLET_GOING_LEFT : 0);
				}
				
				set_object_position(&enemyAttribs[i], enemies[i].position[0], enemies[i].position[1]);
			}
			
			for(int i = 0; i < bulletCount; i++){
				bullets[i].position[0] += (bullets[i].flags & BULLET_GOING_LEFT ? -1 : 1);
				
				if(bullets[i].position[0] < -player_width || bullets[i].position[0] > SCREEN_WIDTH + player_width){
					RemoveBullet(&bullets[i]);
					i--;
					continue;
				}
				
				if(bullets[i].flags & BULLET_IS_PLAYER){
					int bulletRemoved = 0;
					for(int enemyIdx = 0; enemyIdx < enemyCount; enemyIdx++){
						if(abs(bullets[i].position[0] - enemies[enemyIdx].position[0] + 4) < 8
						&& abs(bullets[i].position[1] - enemies[enemyIdx].position[1] + 4) < 8){
							RemoveEnemy(&enemies[enemyIdx]);
							bulletRemoved = 1;
						}
					}
					
					if(bulletRemoved){
						RemoveBullet(&bullets[i]);
						i--;
						continue;
					}
				}
				else{
					//player health down?
					if(abs(bullets[i].position[0] - player_x + 4) < 8
					&& abs(bullets[i].position[1] - player_y + 4) < 8){
						player_x = 5.0f, player_y = 96.0f;
						REG_SND_CH1_FRQ = SFREQ_RESET | SND_RATE(4246, 2);
						LoadLevel(levels[levelIdx]);
						break;
					}
				}
				
				set_object_position(&bulletAttribs[i], bullets[i].position[0], bullets[i].position[1]);
			}
			
			if(enemyCount == 0){
				if(levels[levelIdx].reachedGoal == 0){
					REG_SND_CH1_FRQ = SFREQ_RESET | SND_RATE(7144, 2);
					PopText();
					PushText("fab fab", 8, 150);
					levels[levelIdx].reachedGoal = 1;
				}
			}
			
			if(levels[levelIdx].reachedGoal == 1){
				if(abs(player_x - levels[levelIdx].goalPos[0]) < 8
				&& abs(player_y - levels[levelIdx].goalPos[1]) < 8){
					levels[levelIdx].reachedGoal = 2;
					set_sprite_memory(*(goalUISprites[levelIdx]), (uint16 *)tile_memory[4][9+levelIdx]);
					goalsReached |= (1 << levelIdx);
					REG_SND_CH1_FRQ = SFREQ_RESET | SND_RATE(5666, 2);
				}
			}
			
			if(levels[levelIdx].reachedGoal == 1){
				set_object_position(goalAttribs, levels[levelIdx].goalPos[0], levels[levelIdx].goalPos[1]);
			}
			else{
				set_object_position(goalAttribs, -20, -20);
			}
			
			if(player_x <= 0 && levelIdx > 0){
				levelIdx--;
				player_x = player_max_clamp_x - 5.0f;
				LoadLevel(levels[levelIdx]);
			}
			else if(player_x >= player_max_clamp_x && levelIdx < ARRAY_LENGTH(levels) - 1){
				levelIdx++;
				player_x = 5.0f;
				LoadLevel(levels[levelIdx]);
			}
		}
		else if(currMode == MENU){
			set_object_position(arrowAttribs, 170, 20+menuIdx*10);
		
			if((key_states & BUTTON_B) && !(prevKeys & BUTTON_B)){
				ExitMenuMode();
			}
			
			if((key_states & KEY_DOWN) && !(prevKeys & KEY_DOWN)){
				menuIdx = (menuIdx + 1) % ARRAY_LENGTH(menuItems);
			}
			else if((key_states & KEY_UP) && !(prevKeys & KEY_UP)){
				menuIdx = (menuIdx - 1) % ARRAY_LENGTH(menuItems);
			}
			
			if(!(key_states & BUTTON_A) && (prevKeys & BUTTON_A)){
				ExecuteMenuAction(menuItems[menuIdx].action);
			}
		}
		
		prevKeys = key_states;
		
	}

	return 0;
}

