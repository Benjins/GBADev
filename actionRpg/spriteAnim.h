typedef struct{
	SpriteAnim animData;
	volatile uint16* tileMemory;
	int timerId;
	int frame;
} AnimationInst;

#ifndef MAX_ANIM_INST_COUNT
#define MAX_ANIM_INST_COUNT 20
#endif

typedef struct{
	AnimationInst anims[MAX_ANIM_INST_COUNT];
	int animCount;
} AnimationList;

void AddAnimation(AnimationList* animList, SpriteAnim animData, volatile uint16* tileMemory, TimerList* timers){
	animList->anims[animList->animCount].animData = animData;
	animList->anims[animList->animCount].tileMemory = tileMemory;
	animList->anims[animList->animCount].frame = 0;
	animList->animCount++;
	
	animList->anims[animList->animCount].timerId = AddTimer(timers, animData.keys[0].duration);
	set_sprite_memory(*animData.keys[0].sprite, tileMemory);
}

void UpdateAnimations(AnimationList* animList, TimerList* timers){
	for(int i = 0; i < animList->animCount; i++){
		if(IsTimerDone(timers, animList->anims[i].timerId)){
			animList->anims[i].frame++;
			
			if(animList->anims[i].frame >= animList->anims[i].animData.keyCount){
				animList->anims[i].frame -= animList->anims[i].animData.keyCount;
			}
			
			animList->anims[i].timerId = AddTimer(timers, animList->anims[i].animData.keys[animList->anims[i].frame].duration);
			set_sprite_memory(*animList->anims[i].animData.keys[animList->anims[i].frame].sprite, animList->anims[i].tileMemory);
			
			int tileIndex = ((int)(animList->anims[i].tileMemory) - (int)(tile_memory[4][0]))/sizeof(tile4bpp);
			int palIdx = animList->anims[i].animData.keys[animList->anims[i].frame].sprite->palIdx;
			for (int i = 0; i < 128; i++){
				if ((oam_memory[i].attribute_two & 0x03FF) == tileIndex){
					oam_memory[i].attribute_two = (oam_memory[i].attribute_two & 0x0FFF) | (palIdx << 12);
				}
			}
		}
	}
}