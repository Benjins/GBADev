#include <gba_interrupt.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef char int8;
typedef short int16;
typedef int int32;

#include "random.h"

#include "Timer.h"

typedef uint16 rgb15;
typedef struct object_attributes {
	uint16 attribute_zero;
	uint16 attribute_one;
	uint16 attribute_two;
	uint16 pad;
} object_attributes __attribute__((aligned(4))) ;
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

#define OBJECT_ATTRIBUTE_ZERO_Y_MASK  0xFF
#define OBJECT_ATTRIBUTE_ONE_X_MASK  0x1FF
#define OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK  0x1000

#define oam_memory ((volatile object_attributes *)MEM_OAM)
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

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

/*[GameEntity("bg3.bmp")]*/
typedef struct {
	int16 position[2];
	const char* whatSay;
} object;

#define MAX_LETTER_COUNT 30
#define MAX_TEXT_BOX_COUNT 31
#define MAX_OBJECT_COUNT 10
#define MAX_MONSTER_COUNT 20

volatile object_attributes* uiTextAttribs = &oam_memory[0];
volatile object_attributes* textBoxAttribs = &oam_memory[MAX_LETTER_COUNT];
volatile object_attributes* playerHealthAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT];
volatile object_attributes* playerAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+1];
volatile object_attributes* objectAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+2];
volatile object_attributes* monsterAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+MAX_OBJECT_COUNT+2];
volatile object_attributes* testSprtAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+MAX_OBJECT_COUNT+MAX_MONSTER_COUNT+2];

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

static inline int signum(int value) { return (value > 0 ? 1 : (value < 0 ? -1 : 0) ); }

static inline float clampf(float value, float min, float max) { return (value < min ? min : (value > max ? max : value)); }

#define MAX_TEXT_STRING_COUNT 16

int letterCount = 0;
int textIndexCount = 0;
int textIndices[MAX_TEXT_STRING_COUNT] = {};

void ClearText(){
	textIndexCount = 0;
	letterCount = 0;
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

void ShowTextBox(){
	int xPos = 0;
	int yPos = SCREEN_HEIGHT - 24;
	for(int i = 0; i < MAX_TEXT_BOX_COUNT; i++){
		set_object_position(&textBoxAttribs[i], xPos, yPos);
		xPos += 16;
		if(xPos > SCREEN_WIDTH){
			xPos = 0;
			yPos += 16;
		}
	}
}

void HideTextBox(){
	for(int i = 0; i < MAX_TEXT_BOX_COUNT; i++){
		set_object_position(&textBoxAttribs[i], -18, -18);
	}
}

void PushText(const char* text, int x, int y){
	static const int caseMask = 'a' ^ 'A';
	
	textIndices[textIndexCount] = letterCount;
	textIndexCount++;
	
	int index = letterCount;
	int position = 0;
	for(const char* cursor = text; *cursor != '\0'; cursor++){
		if(*cursor == ' '){
			position++;
			continue;
		}
		
		char baseLetter = *cursor | caseMask;
		if(baseLetter >= 'a' && baseLetter <= 'z'){
			int spriteIdx = 32 + baseLetter - 'a';
			uiTextAttribs[index].attribute_two = spriteIdx;
			set_object_position(&uiTextAttribs[index], 8*position+x, y);
			index++;
			position++;
		}
	}
	
	letterCount = index;
}

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

#include "spriteAnim.h"

void SetHealthSprite(volatile uint16* memory, int health, int maxHealth){
	for(int i = 0; i < 16; i++){
		memory[i] = 0;
	}
	
	int healthFactor = (health * 16)/maxHealth;
	for(int i = 0; i < healthFactor; i++){
		memory[i/4] |= (1 << (i % 4)*4);
	}
}

int DotProduct(int* v1, int* v2){
	return v1[0]*v2[0] + v1[1]*v2[1];
}

typedef enum {
	UP = 0,
	DOWN,
	LEFT,
	RIGHT,
	DIR_COUNT
} Direction;

static int directionVectors[DIR_COUNT][2] = {{0, 1}, {0, -1}, {-1, 0}, {1, 0}};

typedef enum {
	FREEWALK,
	CONVERSATION,
	COMBAT
} GameMode;

GameMode currMode = FREEWALK;

typedef enum{
	COMBAT_MENU,
	ATTACK_MENU
} CombatSubMode;

CombatSubMode combatMode = COMBAT_MENU;

int currentMonsterFight = -1;
int shouldEnterCombat = 0;
int shouldExitCombat = 0;

TimerList timers = {};
AnimationList anims = {};

#include "monster.h"

Sprite playerDirections[DIR_COUNT] = {playerSpriteUp, playerSpriteDown, playerSpriteLeft, playerSpriteRight};
	
const int player_height = 8, player_width = 8;
	
const int centerX = SCREEN_WIDTH/2 - player_height/2;
const int centerY = SCREEN_HEIGHT/2 - player_width/2;
	
int playerX = 0, playerY = 0;
Direction playerDir = DOWN;

const int maxPlayerHealth = 16;
int playerHealth = maxPlayerHealth;

volatile uint16* player_health_memory = (uint16 *)tile_memory[4][12];

volatile uint16* screenmap0Start = &scr_blk_mem[24][0];

//We set them to -1, so that we force an update (since they are immediately set to 0)
int bgTileOffsetX = -1;
int bgTileOffsetY = -1;

volatile uint16* monster_tile_memory = (uint16 *)tile_memory[4][13];

/*[Level]*/
typedef struct{
	object objects[MAX_OBJECT_COUNT];
	/*[ArrayCount("objects")]*/
	int objectCount;
	
	Monster monsters[MAX_MONSTER_COUNT];
	/*[ArrayCount("monsters")]*/
	int monsterCount;
} Level;

Level level = {};

void AddObject(int x, int y, const char* whatSay){
	level.objects[level.objectCount].position[0] = x;
	level.objects[level.objectCount].position[1] = y;
	level.objects[level.objectCount].whatSay = whatSay;
	
	level.objectCount++;
}

void AddMonster(int x, int y, int health){
	level.monsters[level.monsterCount].position[0] = x;
	level.monsters[level.monsterCount].position[1] = y;
	level.monsters[level.monsterCount].timerId = AddTimer(&timers, (GetRandom() % 20) + 30);
	level.monsters[level.monsterCount].currState = PAUSE;
	level.monsters[level.monsterCount].maxHealth = health;
	level.monsters[level.monsterCount].health = health;
	
	level.monsterCount++;
}

void RemoveMonster(int idx){
	level.monsters[idx] = level.monsters[level.monsterCount-1];
	level.monsterCount--;
}

typedef enum{
	ATTACK,
	RUN,
} CombatOption;

typedef struct{
	const char* text;
	CombatOption effect;
} CombatMenuOption;

const CombatMenuOption combatMenu[] = {{"Attack", ATTACK}, {"Run", RUN}};
int combatMenuIndex = 0;

typedef enum{
	LIGHT_ATTACK = 0,
	HEAVY_ATTACK,
} AttackOption;

typedef struct{
	int baseAccuracy; // Percentage
	int basePower; // damage
	int baseEnergy; // currently unused
} MoveData;

MoveData moveData[] = {{90, 1, 1}, {40, 3, 4}};

typedef struct{
	const char* text;
	AttackOption effect;
} AttackMenuOption;

const AttackMenuOption attackMenu[] = {{"Light Attack", LIGHT_ATTACK}, {"Heavy Attack", HEAVY_ATTACK}};
int attackMenuIndex = 0;

void EnterCombat(){
	if(currentMonsterFight < 0 || currentMonsterFight >= level.monsterCount){
		return;
	}
	
	for(int i = 0; i < 128; i++){
		oam_memory[i].attribute_zero = 0;
		oam_memory[i].attribute_one = 0;
		oam_memory[i].attribute_two = 0;
	}
	
	anims.animCount = 0;
	AddAnimation(&anims, monster_large_anim, monster_tile_memory, &timers);
	
	volatile uint16* arrow_tile_mem = (uint16 *)tile_memory[4][17];
	
	AddAnimation(&anims, arrow_anim, arrow_tile_mem, &timers);
	
	for(int i = 0; i < 32*32; i++){
		screenmap0Start[i] = 0;
	}
	
	objectAttribs[0].attribute_zero = 0; 
	objectAttribs[0].attribute_one = 0x4000; 
	objectAttribs[0].attribute_two = 13;
	
	objectAttribs[1].attribute_zero = 0; 
	objectAttribs[1].attribute_one = 0; 
	objectAttribs[1].attribute_two = 17;
	
	objectAttribs[2].attribute_zero = 0;
	objectAttribs[2].attribute_one = 0; 
	objectAttribs[2].attribute_two = 18;
	
	objectAttribs[3].attribute_zero = 0;
	objectAttribs[3].attribute_one = 0; 
	objectAttribs[3].attribute_two = 19; 
	
	volatile uint16* monster_healthbar_tile_mem = (uint16 *)tile_memory[4][18];
	SetHealthSprite(monster_healthbar_tile_mem, level.monsters[currentMonsterFight].health, level.monsters[currentMonsterFight].maxHealth);
	
	volatile uint16* player_healthbar_tile_mem = (uint16 *)tile_memory[4][19];
	SetHealthSprite(player_healthbar_tile_mem, playerHealth, maxPlayerHealth);
	
	for(int i = 0; i < ARRAY_LENGTH(combatMenu); i++){
		PushText(combatMenu[i].text, SCREEN_WIDTH - 120, SCREEN_HEIGHT - 50 + 10*i);
	}
	
	set_object_position(&objectAttribs[0], SCREEN_WIDTH - 50, 30);
	set_object_position(&objectAttribs[2], 50, 30);
	set_object_position(&objectAttribs[3], 50, 60);
	
	currMode = COMBAT;
	combatMode = COMBAT_MENU;
}

void ExitCombat(){
	for(int i = 0; i < 128; i++){
		oam_memory[i].attribute_zero = 0;
		oam_memory[i].attribute_one = 0;
		oam_memory[i].attribute_two = 0;
	}
	
	ClearText();
	
	volatile uint16* object_tile_memory = (uint16 *)tile_memory[4][1+DIR_COUNT];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { object_tile_memory[i] = 0x2232; }
	
	for(int i = 0; i < 4; i++){
		volatile uint16* text_box_tile_memory = (uint16 *)tile_memory[4][i+2+DIR_COUNT];
		set_sprite_memory(textBoxSprite, text_box_tile_memory);
	}
	
	anims.animCount = 0;
	AddAnimation(&anims, monster_anim, monster_tile_memory, &timers);
	
	for(int i = 0; i < DIR_COUNT; i++){
		volatile uint16* player_tile_memory = (uint16 *)tile_memory[4][i+1];
		set_sprite_memory(playerDirections[i], player_tile_memory);
	}
	
	volatile uint16* palTest1Mem = (uint16 *)tile_memory[4][28];
	set_sprite_memory(palTest1, palTest1Mem);
	
	volatile uint16* palTest2Mem = (uint16 *)tile_memory[4][29];
	set_sprite_memory(palTest2, palTest2Mem);
	
	testSprtAttribs[0].attribute_zero = 0;
	testSprtAttribs[0].attribute_one = 0; 
	testSprtAttribs[0].attribute_two = 28 | (palTest1.palIdx << 12);
	set_object_position(&testSprtAttribs[0], 17, 57);
	
	testSprtAttribs[1].attribute_zero = 0;
	testSprtAttribs[1].attribute_one = 0; 
	testSprtAttribs[1].attribute_two = 29 | (palTest2.palIdx << 12);;
	set_object_position(&testSprtAttribs[1], 27, 37);
	
	
	Sprite font[] = {aFont, bFont, cFont, dFont, eFont, fFont, gFont, hFont, iFont,
					jFont, kFont, lFont, mFont, nFont, oFont, pFont, qFont, rFont, sFont,
					tFont, uFont, vFont, wFont, xFont, yFont, zFont};
					
	static_assert(ARRAY_LENGTH(font) == 26, "Font must have 26 characters.");
	
	for(int i = 0; i < ARRAY_LENGTH(font); i++){
		volatile uint16* uiFontMemory = (uint16 *)tile_memory[4][32+i];
		set_sprite_memory(font[i], uiFontMemory);
	}
	
	for(int i = 0; i < MAX_TEXT_BOX_COUNT; i++){
		volatile object_attributes* textBoxAttrib = &textBoxAttribs[i];
		textBoxAttrib->attribute_zero = 0x0000;
		textBoxAttrib->attribute_one = 0x4000; 
		textBoxAttrib->attribute_two = (2+DIR_COUNT);
		set_object_position(textBoxAttrib, -17, -17);
	}
	
	playerHealthAttribs->attribute_zero = 0;
	playerHealthAttribs->attribute_one = 0;
	playerHealthAttribs->attribute_two = 12;
	
	SetHealthSprite(player_health_memory, playerHealth, maxPlayerHealth);
	set_object_position(playerHealthAttribs, 16, 16);
	
	playerAttribs->attribute_zero = 0; 
	playerAttribs->attribute_one = 0; 
	playerAttribs->attribute_two = (1+playerDir) | (playerDirections[playerDir].palIdx << 12);
	set_object_position(playerAttribs, centerX, centerY);
	
	for(int i = 0; i < MAX_OBJECT_COUNT; i++){
		volatile object_attributes* objectAttrib = &objectAttribs[i];
		objectAttrib->attribute_zero = 0; 
		objectAttrib->attribute_one = 0; 
		objectAttrib->attribute_two = 5;
		set_object_position(objectAttrib, -10, -10);
	}
	
	for(int i = 0; i < MAX_MONSTER_COUNT; i++){
		volatile object_attributes* monsterAttrib = &monsterAttribs[i];
		monsterAttrib->attribute_zero = 0; 
		monsterAttrib->attribute_one = 0; 
		monsterAttrib->attribute_two = 13;
		set_object_position(monsterAttrib, -10, -10);
	}
	
	bgTileOffsetX--;
	bgTileOffsetY--;
	
	currMode = FREEWALK;
}

int main(void) {
	irqInit();
	irqEnable(IRQ_VBLANK);
	
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		bg0_palette_memory[i] = paletteColors[i];
		object_palette_memory[i] = paletteColors[i];
	}
	
	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }
	
	ExitCombat();
	
	AddObject(-50, 50, "I am you");
	AddObject(-20, 150, "The only way");
	AddObject(210, -20, "We are one");
	AddObject(120, 70, "lololol");
	AddObject(180, 90, "god is dead");
	AddObject(70, 120, "who is john galt");
	
	AddMonster(-20, 90, 20);
	AddMonster(50, 190, 22);
	AddMonster(120, 50, 23);
	AddMonster(20, 50, 18);
		
	// Set the display parameters to enable objects, and use a 1D object->tile mapping, and enable BG0
	REG_DISPLAY = 0x1000 | 0x0040 | 0x0100;
	
	REG_BG0_CNT = BG_CBB(0) | BG_SBB(24) | BG_REG_64x32;
	
	volatile uint16* bg0_tile_mem0 = (uint16 *)tile_memory[0][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) {bg0_tile_mem0[i] = 0;}
	
	for(int i = 0; i < backMap.bgCount; i++){
		volatile uint16* bg0_tile_mem = (uint16 *)tile_memory[0][i+1];
	
		set_sprite_memory(backMap.bgSprites[i], bg0_tile_mem);
	}
	
	uint32 prevKeys = 0;
	
	HideTextBox();
	
	int combatId = -1;
	
	while (1) {
		asm("swi 0x05");
		
		if(shouldEnterCombat){
			EnterCombat();
			shouldEnterCombat = 0;
		}
		if(shouldExitCombat){
			ExitCombat();
			shouldExitCombat = 0;
		}
		
		UpdateTimers(&timers);
		UpdateAnimations(&anims, &timers);
		
		uint32 key_states = ~REG_KEY_INPUT & KEY_ANY;
		
		if(currMode == FREEWALK){		
			int player_max_clamp_y = SCREEN_HEIGHT - player_height;
			int player_max_clamp_x = SCREEN_WIDTH - player_width;
			
			int newPlayerX = playerX;
			int newPlayerY = playerY;
			
			if (key_states & KEY_LEFT) {
				newPlayerX--;
				playerDir = LEFT;
			}
			if (key_states & KEY_RIGHT) {
				newPlayerX++;
				playerDir = RIGHT;
			}
			if (key_states & KEY_UP) {
				newPlayerY--;
				playerDir = UP;
			}
			if (key_states & KEY_DOWN) {
				newPlayerY++;
				playerDir = DOWN;
			}
			
			playerAttribs->attribute_two = 1+playerDir;
			
			int playerTileX = (playerX + SCREEN_WIDTH/2)/8;
			int playerTileY = (playerY + SCREEN_HEIGHT/2)/8;
			
			int newPlayerTileX = (newPlayerX + SCREEN_WIDTH/2)/8;
			int newPlayerTileY = (newPlayerY + SCREEN_HEIGHT/2)/8;
			
			if(newPlayerTileX == playerTileX){
				playerX = newPlayerX;
			}
			else{
				int newTileIdx = newPlayerTileX + backMap.map.width*playerTileY;
				int tileID = backMap.map.data[newTileIdx]-1;
				if(backMap.spriteFlags[tileID] & 0x01){
					playerX = newPlayerX;
				}
			}
			
			if(newPlayerTileY == playerTileY){
				playerY = newPlayerY;
			}
			else{
				int newTileIdx = playerTileX + backMap.map.width*newPlayerTileY;
				int tileID = backMap.map.data[newTileIdx]-1;
				if(backMap.spriteFlags[tileID] & 0x01){
					playerY = newPlayerY;
				}
			}
			
			int BGTileOffsetX = playerX/8;
			int BGTileOffsetY = playerY/8;
			
			int bgOffsetX = playerX%8;
			int bgOffsetY = playerY%8;
			
			if(bgOffsetX < 0){
				bgOffsetX += 8;
				BGTileOffsetX--;
			}
			
			if(bgOffsetY < 0){
				bgOffsetY += 8;
				BGTileOffsetY--;
			}
			
			REG_BG0_OFS[0] = bgOffsetX;
			REG_BG0_OFS[1] = bgOffsetY;
			
			if((BGTileOffsetX != bgTileOffsetX) || (BGTileOffsetY != bgTileOffsetY)){		
				bgTileOffsetX = BGTileOffsetX;
				bgTileOffsetY = BGTileOffsetY;
				
				for(int j = 0; j < 32; j++){
					for(int i = 0; i < 32; i++){
						int backMapX = (i+bgTileOffsetX)%backMap.map.width;
						int backMapY = (j+bgTileOffsetY)%backMap.map.height;
						if(backMapX < 0){
							backMapX += backMap.map.width;
						}
						if(backMapY < 0){
							backMapY += backMap.map.height;
						}
						int backMapIdx = backMapY*backMap.map.width+backMapX;
						
						int scrMapIdx = j*32+i;
						screenmap0Start[scrMapIdx] = backMap.map.data[backMapIdx] | (backMap.bgSprites[backMap.map.data[backMapIdx] - 1].palIdx << 12);
					}
				}
			}
			
			UpdateMonsters(level.monsters, level.monsterCount, &timers, playerX, playerY, &playerHealth);
			
			SetHealthSprite(player_health_memory, playerHealth, maxPlayerHealth);
			
			for(int i = 0; i < level.monsterCount; i++){
				volatile object_attributes* monsterAttrib = &monsterAttribs[i];
				
				int screenX = level.monsters[i].position[0] - playerX + centerX;
				int screenY = level.monsters[i].position[1] - playerY + centerY;
				
				if(screenX < -10 || screenX > SCREEN_WIDTH  + 10
				|| screenY < -10 || screenY > SCREEN_HEIGHT + 10){
					set_object_position(monsterAttrib, -10, -10);
				}
				else{
					set_object_position(monsterAttrib, screenX, screenY);
				}
			}
				
			for(int i = 0; i < level.objectCount; i++){
				volatile object_attributes* objectAttrib = &objectAttribs[i];
				int screenX = level.objects[i].position[0] - playerX + centerX;
				int screenY = level.objects[i].position[1] - playerY + centerY;
				
				//Prevent wrap-around from the truncation that set_pos uses.
				//TODO: Use clamp instead?
				if(screenX < -10 || screenX > SCREEN_WIDTH  + 10
				|| screenY < -10 || screenY > SCREEN_HEIGHT + 10){
					set_object_position(objectAttrib, -10, -10);
				}
				else{
					set_object_position(objectAttrib, screenX, screenY);
				}
			}
			
			if((key_states & BUTTON_A) && !(prevKeys & BUTTON_A)){
				for(int i = 0; i < level.objectCount; i++){
					int diffX = level.objects[i].position[0] - playerX;
					int diffY = level.objects[i].position[1] - playerY;
					
					int diffSqr = diffX*diffX + diffY*diffY;
					
					if(diffSqr < 128){
						PushText(level.objects[i].whatSay, 5, SCREEN_HEIGHT - 20);
						ShowTextBox();
						currMode = CONVERSATION;
					}
				}
			}
		}
		else if(currMode == CONVERSATION){
			if((key_states & BUTTON_B) && !(prevKeys & BUTTON_B)){
				PopText();
				HideTextBox();
				currMode = FREEWALK;
			}
		}
		else if(currMode == COMBAT){
			UpdateTimers(&timers);
			
			if (combatMode == COMBAT_MENU){
				if((key_states & KEY_DOWN) && !(prevKeys & KEY_DOWN)){
					combatMenuIndex = (combatMenuIndex + 1) % ARRAY_LENGTH(combatMenu);
				}
				if((key_states & KEY_UP) && !(prevKeys & KEY_UP)){
					combatMenuIndex = (combatMenuIndex + ARRAY_LENGTH(combatMenu) - 1) % ARRAY_LENGTH(combatMenu);
				}
				
				if(!(key_states & BUTTON_A) && (prevKeys & BUTTON_A)){
					CombatOption effect = combatMenu[combatMenuIndex].effect;
					
					if(effect == ATTACK){
						combatMode = ATTACK_MENU;
						
						for(int i = 0; i < ARRAY_LENGTH(combatMenu); i++){
							PopText();
						}
						
						for(int i = 0; i < ARRAY_LENGTH(attackMenu); i++){
							PushText(attackMenu[i].text, SCREEN_WIDTH - 120, SCREEN_HEIGHT - 50 + 10*i);
						}
					}
					else if(effect == RUN){
						shouldExitCombat = 1;
					}
				}
				
				set_object_position(&objectAttribs[1], SCREEN_WIDTH - 130, SCREEN_HEIGHT - 50 + 10*combatMenuIndex);
			}
			else if (combatMode == ATTACK_MENU){
				if((key_states & KEY_DOWN) && !(prevKeys & KEY_DOWN)){
					attackMenuIndex = (attackMenuIndex + 1) % ARRAY_LENGTH(attackMenu);
				}
				if((key_states & KEY_UP) && !(prevKeys & KEY_UP)){
					attackMenuIndex = (attackMenuIndex + ARRAY_LENGTH(attackMenu) - 1) % ARRAY_LENGTH(attackMenu);
				}
				
				if(!(key_states & BUTTON_A) && (prevKeys & BUTTON_A)){
					AttackOption effect = attackMenu[attackMenuIndex].effect;
					
					playerHealth--;
					volatile uint16* player_healthbar_tile_mem = (uint16 *)tile_memory[4][19];
					SetHealthSprite(player_healthbar_tile_mem, playerHealth, maxPlayerHealth);
					
					int rng = GetRandom() % 100;
					if (rng < moveData[effect].baseAccuracy){
						level.monsters[currentMonsterFight].health -= moveData[effect].basePower;
						
						volatile uint16* healthbar_tile_mem = (uint16 *)tile_memory[4][18];
						SetHealthSprite(healthbar_tile_mem, level.monsters[currentMonsterFight].health, level.monsters[currentMonsterFight].maxHealth);
					}
					
					if(level.monsters[currentMonsterFight].health <= 0){
						RemoveMonster(currentMonsterFight);
						shouldExitCombat = 1;
					}
				}
				
				if(!(key_states & BUTTON_B) && (prevKeys & BUTTON_B)){
					combatMode = COMBAT_MENU;
					
					for(int i = 0; i < ARRAY_LENGTH(attackMenu); i++){
						PopText();
					}
					
					for(int i = 0; i < ARRAY_LENGTH(combatMenu); i++){
						PushText(combatMenu[i].text, SCREEN_WIDTH - 120, SCREEN_HEIGHT - 50 + 10*i);
					}
				}
				
				set_object_position(&objectAttribs[1], SCREEN_WIDTH - 130, SCREEN_HEIGHT - 50 + 10*attackMenuIndex);
			}
		}
		
		prevKeys = key_states;
		
	}

	return 0;
}