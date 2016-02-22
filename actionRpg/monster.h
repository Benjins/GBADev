static int fightDistSqr = 8*8;
static int chaseBeginDistSqr = 40*40;
static int chaseEndDistSqr = 50*50;

typedef enum{
	PATROL_FIRST,
	PATROL_UP = PATROL_FIRST,
	PATROL_DOWN,
	PATROL_LEFT,
	PATROL_RIGHT,
	PATROL_LAST = PATROL_RIGHT,
	PAUSE,
	SEEK,
	FIGHT
} MonsterState;

typedef struct{
	int position[2];
	MonsterState currState;
	int timerId;
} Monster;

inline void ResolveCollisions(int* startPos, int* moveDir, int* outPos){
	static const int offsetVec[2] = {SCREEN_WIDTH/2, SCREEN_HEIGHT/2};
	
	for(int j = 0; j < 2; j++){
		int currPos = startPos[j] + offsetVec[j];
		int nextPos = currPos + moveDir[j];
		
		int currOtherTile = (startPos[1 - j] + offsetVec[1 - j]) / 8;
		int currTile = currPos / 8;
		int nextTile = nextPos / 8;
		if(currTile == nextTile){
			outPos[j] = nextPos - offsetVec[j];
		}
		else{
			int newTileIdx = nextTile + backMap.map.width*currOtherTile;
			if(j == 1){
				newTileIdx = currOtherTile + backMap.map.width*nextTile;
			}
			
			int tileID = backMap.map.data[newTileIdx]-1;
			if(backMap.spriteFlags[tileID] & 0x01){
				outPos[j] = nextPos - offsetVec[j];
			}
		}
	}
}

void UpdateMonsters(Monster* monsters, int monsterCount, TimerList* timers, int playerX, int playerY, int* playerHealth){
	int playerPos[2] = {playerX, playerY};
	
	for(int i = 0; i < monsterCount; i++){
		int playerDir[2] = {playerPos[0] - monsters[i].position[0], playerPos[1] - monsters[i].position[1]};
		int playerDistSqr = playerDir[0]*playerDir[0] + playerDir[1]*playerDir[1];
		
		if(monsters[i].currState >= PATROL_FIRST && monsters[i].currState <= PATROL_LAST){
			int newPos[2] = {monsters[i].position[0], monsters[i].position[1]};
			ResolveCollisions(monsters[i].position, directionVectors[monsters[i].currState - PATROL_FIRST], newPos);
			
			for(int j = 0; j < 2; j++){
				monsters[i].position[j] = newPos[j];
			}
			
			if(IsTimerDone(timers, monsters[i].timerId)){
				monsters[i].timerId = AddTimer(timers, (GetRandom() % 20) + 30);
				monsters[i].currState = PAUSE;
			}
			
			if(playerDistSqr <= chaseBeginDistSqr){
				monsters[i].currState = SEEK;
			}
		}
		else if(monsters[i].currState == PAUSE){
			if(IsTimerDone(timers, monsters[i].timerId)){
				monsters[i].timerId = AddTimer(timers, (GetRandom() % 20) + 30);
				monsters[i].currState = (MonsterState)(PATROL_FIRST + (GetRandom() % (PATROL_LAST - PATROL_FIRST)));
			}
			
			if(playerDistSqr <= chaseBeginDistSqr){
				monsters[i].currState = SEEK;
			}
		}
		else if(monsters[i].currState == SEEK){
			int moveVec[2] = {};
			if(playerDistSqr <= fightDistSqr){
				monsters[i].currState = FIGHT;
				monsters[i].timerId = AddTimer(timers, 20);
			}
			else if(playerDistSqr >= chaseEndDistSqr){
				monsters[i].timerId = AddTimer(timers, 20);
				monsters[i].currState = (MonsterState)(PATROL_FIRST + (GetRandom() % PATROL_LAST - PATROL_FIRST));
			}
			else if(abs(playerDir[0]) > abs(playerDir[1])){
				moveVec[0] = signum(playerDir[0]);
			}
			else{
				moveVec[1] = signum(playerDir[1]);
			}
			
			int newPos[2] = {monsters[i].position[0], monsters[i].position[1]};
			ResolveCollisions(monsters[i].position, moveVec, newPos);
			
			for(int j = 0; j < 2; j++){
				monsters[i].position[j] = newPos[j];
			}
		}
		else if(monsters[i].currState == FIGHT){
			if(IsTimerDone(timers, monsters[i].timerId)){
				if(playerDistSqr <= fightDistSqr){
					shouldEnterCombat = 1;
				}
				
				monsters[i].currState = SEEK;
			}
			
		}
		else{
			//Shouldn't happen
		}
	}
}