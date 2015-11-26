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

#define OBJECT_ATTRIBUTE_ZERO_Y_MASK  0xFF
#define OBJECT_ATTRIBUTE_ONE_X_MASK  0x1FF
#define OBJECT_ATTRIBUTE_ONE_HORIZONTAL_FLIP_MASK  0x1000

#define oam_memory ((volatile object_attributes *)MEM_OAM)
#define tile_memory ((volatile tile_block *)MEM_VRAM)
#define object_palette_memory ((volatile rgb15 *)(MEM_PAL + 0x200))

#define ARRAY_LENGTH(x) (sizeof(x)/sizeof((x)[0]))

// Form a 16-bit BGR GBA colour from three component values (hopefully, in range).
static inline rgb15 RGB15(int r, int g, int b) { return r | (g << 5) | (b << 10); }

// Set the position of an object to specified x and y coordinates (hopefully, in range).
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

static inline void set_sprite_memory(Sprite sprite, volatile uint16* memory){
	for(int j = 0; j < sprite.height; j++){
		for(int i = 0; i < sprite.width; i++){
			int spriteIdx = j * sprite.width  + i;
			int memIdx  = ((i / 8) * 64) + (sprite.height - 1 - j) * 8 + (i % 8);
			memory[memIdx / 4] |= (sprite.data[spriteIdx] << ((spriteIdx % 4)*4));
		}
	}
}


#define MAX_PLATFORM_COUNT 10
#define MAX_BULLET_COUNT 48
#define MAX_ENEMY_COUNT 8

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

bullet bullets[MAX_BULLET_COUNT];
platform platforms[MAX_PLATFORM_COUNT];
enemy enemies[MAX_ENEMY_COUNT];

int bulletCount = 0;
int platformCount = 0;
int enemyCount = 0;

volatile object_attributes* enemyAttribs = &oam_memory[MAX_PLATFORM_COUNT + 1];
volatile object_attributes* bulletAttribs = &oam_memory[MAX_PLATFORM_COUNT + MAX_ENEMY_COUNT + 2];

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

int main(void) {
	// Write the tiles for our sprites into the 4th tile block in VRAM.
	// Particularly, four tiles for an 8x32 paddle sprite, and 1 tile for an 8x8 ball sprite.
	// 0x1111 = 0001000100010001 [4bpp = colour index 1, colour index 1, colour index 1, colour index 1]
	// 0x2222 = 0002000200020002 [4bpp = colour index 2, colour index 2, colour index 2, colour index 2]
	// NOTE: We're using our own memory writing code here to avoid the byte-granular writes that
	// something like 'memset' might make (GBA VRAM doesn't support byte-granular writes).
	
	volatile uint16 *paddle_tile_memory = (uint16 *)tile_memory[4][1];
	set_sprite_memory(platformSprite, paddle_tile_memory);
	
	volatile uint16 *ball_tile_memory = (uint16 *)tile_memory[4][5];
	for (int i = 0; i < (sizeof(tile4bpp) / 2); ++i) { ball_tile_memory[i] = 0x2222; }
	
	volatile uint16 *ball_player_memory = (uint16 *)tile_memory[4][6];
	set_sprite_memory(ballSprite, ball_player_memory);
	
	volatile uint16 *enemy_memory = (uint16 *)tile_memory[4][7];
	set_sprite_memory(enemySprite, enemy_memory);
	//for (int i = 0; i < (sizeof(tile4bpp) / 2); ++i) { enemy_memory[i] = 0x3003; }
	
	volatile uint16 *bullet_memory = (uint16 *)tile_memory[4][8];
	set_sprite_memory(bulletSprite, bullet_memory);
	
	// Write the colour palette for our sprites into the first palette of
	// 16 colours in colour palette memory (this palette has index 0).
	for(int i = 0; i < ARRAY_LENGTH(paletteColors); i++){
		object_palette_memory[i] = paletteColors[i];
	}
	
	int platInit[][2] = {{20,100}, {40,130}, {70,70}, {70,122}, {120,52}, {86,130}, {10,22}};
	
	for(int i = 0; i < ARRAY_LENGTH(platInit); i++){
		platform* newPlat = AddPlatform();
		newPlat->position[0] = platInit[i][0];
		newPlat->position[1] = platInit[i][1];
	}
	
	int enemyInit[][2] = {{40,122}, {70,70}};
	
	for(int i = 0; i < ARRAY_LENGTH(enemyInit); i++){
		enemy* newEnemy = AddEnemy();
		newEnemy->position[0] = enemyInit[i][0];
		newEnemy->position[1] = enemyInit[i][1];
	}
	
	volatile object_attributes* platformAttribs = &oam_memory[1];
	
	for(int i = 0; i < MAX_PLATFORM_COUNT; i++){
		volatile object_attributes* platformAttrib = &platformAttribs[i];
		platformAttrib->attribute_zero = 0x4000; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
		platformAttrib->attribute_one = 0x4000; // This sprite has a size of 8x8 when the SQUARE shape is set.
		platformAttrib->attribute_two = 1; // This sprite's base tile is the fifth tile in tile block 4, and this sprite should use colour palette 0.
		
		if(i >= platformCount){
			set_object_position(platformAttrib, -10, -10);
		}
	}
	
	for(int i = 0; i < MAX_ENEMY_COUNT; i++){
		volatile object_attributes* enemyAttrib = &enemyAttribs[i];
		enemyAttrib->attribute_zero = 0; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
		enemyAttrib->attribute_one = 0; // This sprite has a size of 8x8 when the SQUARE shape is set.
		enemyAttrib->attribute_two = 7;
		
		if(i >= enemyCount){
			set_object_position(enemyAttrib, -10, -10);
		}
	}
	
	for(int i = 0; i < MAX_BULLET_COUNT; i++){
		volatile object_attributes* bulletAttrib = &bulletAttribs[i];
		bulletAttrib->attribute_zero = 0; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
		bulletAttrib->attribute_one = 0; // This sprite has a size of 8x8 when the SQUARE shape is set.
		bulletAttrib->attribute_two = 8;
		set_object_position(bulletAttrib, -10, -10);
	}
	
	volatile object_attributes* playerAttr = &oam_memory[0];
	playerAttr->attribute_zero = 0; // This sprite is made up of 4bpp tiles and has the SQUARE shape.
	playerAttr->attribute_one = 0; // This sprite has a size of 8x8 when the SQUARE shape is set.
	playerAttr->attribute_two = 6; // This sprite's base tile is the fifth tile in tile block 4, and this sprite should use colour palette 0.

	// Initialize our variables to keep track of the state of the paddle and ball,
	// and set their initial positions (by modifying their attributes in OAM).
	const int player_width = 8, player_height = 8, ball_width = 8, ball_height = 8, platform_width = 32, platform_height = 8;
	const float playerSpeed = 1.8f;
	float player_x = 5.0f, player_y = 96.0f;
	float playerVel = 0;
	int isGrounded = 0;
	int isFlip=0;
	int prevKeys = 0;
	
	set_object_position(playerAttr, (int)(player_x+0.5f), (int)(player_y+0.5f));
	
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
		
		enemies[i].flags = enemies[i].flags | (ENEMY_PLATFORM_MASK & closestPlatIdx);
		
		set_object_position(&enemyAttribs[i], enemies[i].position[0], enemies[i].position[1]);
	}
	
	for(int i = 0; i < bulletCount; i++){
		set_object_position(&bulletAttribs[i], bullets[i].position[0], bullets[i].position[1]);
	}
	
	// Set the display parameters to enable objects, and use a 1D object->tile mapping.
	REG_DISPLAY = 0x1000 | 0x0040;

	// Our main game loop
	uint32 key_states = 0;
	while (1) {
		// Skip past the rest of any current V-Blank, then skip past the V-Draw
		while(REG_DISPLAY_VCOUNT != 160);
		
		// Get current key states (REG_KEY_INPUT stores the states inverted)
		key_states = ~REG_KEY_INPUT & KEY_ANY;

		//Trip out
		//object_palette_memory[3]++;
		
		// Note that our physics update is tied to the framerate rather than a fixed timestep.
		int player_max_clamp_y = SCREEN_HEIGHT - player_height;
		int player_max_clamp_x = SCREEN_WIDTH - player_width;
		
		float new_player_x = player_x;
		
		if ((key_states & KEY_UP) && isGrounded) { playerVel = 130.0f;}
		if (key_states & KEY_LEFT) { new_player_x = clampf(player_x - playerSpeed, 0, player_max_clamp_x);isFlip=1;}
		if (key_states & KEY_RIGHT) { new_player_x = clampf(player_x + playerSpeed, 0, player_max_clamp_x);isFlip=0;}
		
		if((key_states & BUTTON_A) && !(prevKeys & BUTTON_A)){
			bullet* newBullet = AddBullet();
			newBullet->position[0] = player_x;
			newBullet->position[1] = player_y;
			newBullet->flags = BULLET_IS_PLAYER | (isFlip ? BULLET_GOING_LEFT : 0);
		}
		
		prevKeys = key_states;
		
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
					object_palette_memory[1] = RGB15(0x0F, 0x0F, 0x0F);
					RemoveBullet(&bullets[i]);
					i--;
					continue;
				}
			}
			
			set_object_position(&bulletAttribs[i], bullets[i].position[0], bullets[i].position[1]);
		}
		
	}

	return 0;
}