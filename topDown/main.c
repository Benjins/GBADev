#include "../common/gba.h"
#include "../common/random.h"

typedef struct {
	int16 position[2];
	const char* whatSay;
} object;

#define MAX_OBJECT_COUNT 10
object objects[MAX_OBJECT_COUNT];

int objectCount = 0;

void AddObject(int x, int y, const char* whatSay){
	objects[objectCount].position[0] = x;
	objects[objectCount].position[1] = y;
	objects[objectCount].whatSay = whatSay;
	
	objectCount++;
}

#define MAX_LETTER_COUNT 30
#define MAX_TEXT_BOX_COUNT 31
#define MAX_MONSTER_COUNT 20

volatile object_attributes* uiTextAttribs = &oam_memory[0];
volatile object_attributes* textBoxAttribs = &oam_memory[MAX_LETTER_COUNT];
volatile object_attributes* playerHealthAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT];
volatile object_attributes* playerAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+1];
volatile object_attributes* objectAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+2];
volatile object_attributes* monsterAttribs = &oam_memory[MAX_LETTER_COUNT+MAX_TEXT_BOX_COUNT+MAX_OBJECT_COUNT+2];

#define MAX_TEXT_STRING_COUNT 16

int letterCount = 0;
int textIndexCount = 0;
int textIndices[MAX_TEXT_STRING_COUNT] = {};

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
	int position=0;
	for(const char* cursor = text; *cursor != '\0'; cursor++){
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

void SetHealthSprite(volatile uint16* memory, int health){
	for(int i = 0; i < 16; i++){
		memory[i] = 0;
	}
	
	for(int i = 0; i < health; i++){
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
	CONVERSATION
} GameMode;

GameMode currMode = FREEWALK;

#include "monster.h"

#define MAX_MONSTER_COUNT 20

Monster monsters[MAX_MONSTER_COUNT];
int monsterCount = 0;

void AddMonster(int x, int y){
	monsters[monsterCount].position[0] = x;
	monsters[monsterCount].position[1] = y;
	monsters[monsterCount].timer = 0;
	monsters[monsterCount].currState = PATROL_UP;
	
	monsterCount++;
}

int main(void) {
	INT_VECTOR = InterruptMain;
	
	BNS_REG_IME	= 0;
	REG_DISPSTAT |= LCDC_VBL;
	BNS_REG_IE |= IRQ_VBLANK;
	BNS_REG_IME	= 1;
	
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		bg0_palette_memory[i] = paletteColors[i];
		//bg1_palette_memory[i] = paletteColors[i];
		object_palette_memory[i] = paletteColors[i];
	}
	
	Sprite playerDirections[DIR_COUNT] = {playerSpriteUp, playerSpriteDown, playerSpriteLeft, playerSpriteRight};
	
	volatile uint16* empty_tile_memory = (uint16 *)tile_memory[4][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { empty_tile_memory[i] = 0x0000; }
	
	for(int i = 0; i < DIR_COUNT; i++){
		volatile uint16* player_tile_memory = (uint16 *)tile_memory[4][i+1];
		set_sprite_memory(playerDirections[i], player_tile_memory);
	}
	
	volatile uint16* object_tile_memory = (uint16 *)tile_memory[4][1+DIR_COUNT];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) { object_tile_memory[i] = 0x2232; }
	
	Sprite font[] = {aFont, bFont, cFont, dFont, eFont, fFont, gFont, hFont, iFont,
					jFont, kFont, lFont, mFont, nFont, oFont, pFont, qFont, rFont, sFont,
					tFont, uFont, vFont, wFont, xFont, yFont, zFont};
					
	static_assert(ARRAY_LENGTH(font) == 26, "Font must have 26 characters.");
	
	for(int i = 0; i < 4; i++){
		volatile uint16* text_box_tile_memory = (uint16 *)tile_memory[4][i+2+DIR_COUNT];
		set_sprite_memory(textBoxSprite, text_box_tile_memory);
	}
	
	volatile uint16* player_health_memory = (uint16 *)tile_memory[4][12];
	
	volatile uint16* monster_tile_memory = (uint16 *)tile_memory[4][13];
	set_sprite_memory(monsterSprite, monster_tile_memory);
	
	for(int i = 0; i < ARRAY_LENGTH(font); i++){
		volatile uint16* uiFontMemory = (uint16 *)tile_memory[4][14+i];
		set_sprite_memory(font[i], uiFontMemory);
	}
	
	int playerX = 0, playerY = 0;
	Direction playerDir = DOWN;
	int playerHealth = 16;
	
	const int player_height = 8, player_width = 8;
	
	const int centerX = SCREEN_WIDTH/2 - player_height/2;
	const int centerY = SCREEN_HEIGHT/2 - player_width/2;
	
	playerHealthAttribs->attribute_zero = 0;
	playerHealthAttribs->attribute_one = 0;
	playerHealthAttribs->attribute_two = 12;
	
	SetHealthSprite(player_health_memory, playerHealth);
	set_object_position(playerHealthAttribs, 16, 16);
	
	playerAttribs->attribute_zero = 0; 
	playerAttribs->attribute_one = 0; 
	playerAttribs->attribute_two = 1+playerDir;
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
	
	AddObject(-50, 50, "I am you");
	AddObject(-20, 150, "The only way");
	AddObject(210, -20, "We are one");
	AddObject(120, 70, "lololol");
	AddObject(180, 90, "god is dead");
	AddObject(70, 120, "who is john galt");
	
	AddMonster(-20, 90);
	AddMonster(50, 190);
	AddMonster(120, 50);
	AddMonster(20, 50);
	
	for(int i = 0; i < MAX_TEXT_BOX_COUNT; i++){
		volatile object_attributes* textBoxAttrib = &textBoxAttribs[i];
		textBoxAttrib->attribute_zero = 0x0000;
		textBoxAttrib->attribute_one = 0x4000; 
		textBoxAttrib->attribute_two = 2+DIR_COUNT;
	}
		
	// Set the display parameters to enable objects, and use a 1D object->tile mapping, and enable BG0
	REG_DISPLAY = 0x1000 | 0x0040 | 0x0100 ;// | 0x0200;
	
	REG_BG0_CNT = BG_CBB(0) | BG_SBB(24) | BG_REG_64x32;
	//REG_BG1_CNT = BG_CBB(1) | BG_SBB(6) | BG_REG_64x32;
	
	//uint16 bg1_tileCols[] = {0x3421, 0x2333, 0x1220, 0x1001, 0x2333, 0x2302, 0x4424, 0x4112, 0x2314, 0x2222};
	
	volatile uint16* bg0_tile_mem0 = (uint16 *)tile_memory[0][0];
	for (int i = 0; i < (sizeof(tile4bpp) / 2) * 4; ++i) {bg0_tile_mem0[i] = 0;}
	
	for(int i = 0; i < backMap.bgCount; i++){
		volatile uint16* bg0_tile_mem = (uint16 *)tile_memory[0][i+1];
	
		set_sprite_memory(backMap.bgSprites[i], bg0_tile_mem);
	}
	
	volatile uint16* screenmap0Start = &scr_blk_mem[24][0];
	
	uint32 prevKeys = 0;
	
	//We set them to -1, so that we force an update (since they are immediately set to 0)
	int bgTileOffsetX = -1;
	int bgTileOffsetY = -1;
	
	HideTextBox();
	
	while (1) {
		//VBlankIntrWait();
		asm("swi 0x05");
		
		uint32 key_states = ~REG_KEY_INPUT & KEY_ANY;
		
		if(currMode == FREEWALK){		
			int player_max_clamp_y = SCREEN_HEIGHT - player_height;
			int player_max_clamp_x = SCREEN_WIDTH - player_width;
			
			if (key_states & KEY_LEFT) {
				playerX--;
				playerDir = LEFT;
			}
			if (key_states & KEY_RIGHT) {
				playerX++;
				playerDir = RIGHT;
			}
			if (key_states & KEY_UP) {
				playerY--;
				playerDir = UP;
			}
			if (key_states & KEY_DOWN) {
				playerY++;
				playerDir = DOWN;
			}
			
			playerAttribs->attribute_two = 1+playerDir;
			
			int bgOffsetX = playerX%8;
			int bgOffsetY = playerY%8;
			
			int newBGTileOffsetX = playerX/8;
			int newBGTileOffsetY = playerY/8;
			
			if(bgOffsetX < 0){
				bgOffsetX += 8;
				newBGTileOffsetX--;
			}
			
			if(bgOffsetY < 0){
				bgOffsetY += 8;
				newBGTileOffsetY--;
			}
			
			REG_BG0_OFS[0] = bgOffsetX;
			REG_BG0_OFS[1] = bgOffsetY;
			
			if((newBGTileOffsetX != bgTileOffsetX) || (newBGTileOffsetY != bgTileOffsetY)){		
				bgTileOffsetX = newBGTileOffsetX;
				bgTileOffsetY = newBGTileOffsetY;
				
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
						screenmap0Start[scrMapIdx] = backMap.map.data[backMapIdx];
					}
				}
			}
			
			UpdateMonsters(monsters, monsterCount, playerX, playerY, &playerHealth);
			
			SetHealthSprite(player_health_memory, playerHealth);
			
			for(int i = 0; i < monsterCount; i++){
				volatile object_attributes* monsterAttrib = &monsterAttribs[i];
				
				int screenX = monsters[i].position[0] - playerX + centerX;
				int screenY = monsters[i].position[1] - playerY + centerY;
				
				if(screenX < -10 || screenX > SCREEN_WIDTH  + 10
				|| screenY < -10 || screenY > SCREEN_HEIGHT + 10){
					set_object_position(monsterAttrib, -10, -10);
				}
				else{
					set_object_position(monsterAttrib, screenX, screenY);
				}
			}
				
			for(int i = 0; i < objectCount; i++){
				volatile object_attributes* objectAttrib = &objectAttribs[i];
				int screenX = objects[i].position[0] - playerX + centerX;
				int screenY = objects[i].position[1] - playerY + centerY;
				
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
				for(int i = 0; i < objectCount; i++){
					int diffX = objects[i].position[0] - playerX;
					int diffY = objects[i].position[1] - playerY;
					
					int diffSqr = diffX*diffX + diffY*diffY;
					
					if(diffSqr < 128){
						PushText(objects[i].whatSay, 5, SCREEN_HEIGHT - 20);
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
		
		
		
		prevKeys = key_states;
		
	}

	return 0;
}