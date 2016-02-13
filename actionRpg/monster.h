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
	short timer;
	short timerGoal;
} Monster;

void UpdateMonsters(Monster* monsters, int monsterCount, int playerX, int playerY, int* playerHealth){
	int playerPos[2] = {playerX, playerY};
	
	for(int i = 0; i < monsterCount; i++){
		int playerDir[2] = {playerPos[0] - monsters[i].position[0], playerPos[1] - monsters[i].position[1]};
		int playerDistSqr = playerDir[0]*playerDir[0] + playerDir[1]*playerDir[1];
		
		if(monsters[i].currState >= PATROL_FIRST && monsters[i].currState <= PATROL_LAST){
			for(int j = 0; j < 2; j++){
				monsters[i].position[j] += directionVectors[monsters[i].currState][j];
			}
			
			monsters[i].timer++;
			if(monsters[i].timer >= monsters[i].timerGoal){
				monsters[i].timer = 0;
				monsters[i].timerGoal = (GetRandom() % 20) + 30;
				monsters[i].currState = PAUSE;
			}
			
			if(playerDistSqr <= chaseBeginDistSqr){
				monsters[i].currState = SEEK;
			}
		}
		else if(monsters[i].currState == PAUSE){
			monsters[i].timer++;
			
			if(monsters[i].timer >= monsters[i].timerGoal){
				monsters[i].timer = 0;
				monsters[i].timerGoal = (GetRandom() % 20) + 30;
				monsters[i].currState = (MonsterState)(PATROL_FIRST + (GetRandom() % (PATROL_LAST - PATROL_FIRST)));
			}
			
			if(playerDistSqr <= chaseBeginDistSqr){
				monsters[i].currState = SEEK;
			}
		}
		else if(monsters[i].currState == SEEK){
			if(playerDistSqr <= fightDistSqr){
				monsters[i].currState = FIGHT;
				monsters[i].timer = 60;
			}
			else if(playerDistSqr >= chaseEndDistSqr){
				monsters[i].timer = 0;
				monsters[i].currState = (MonsterState)(PATROL_FIRST + (GetRandom() % PATROL_LAST - PATROL_FIRST));
			}
			else if(abs(playerDir[0]) > abs(playerDir[1])){
				monsters[i].position[0] += signum(playerDir[0]);
			}
			else{
				monsters[i].position[1] += signum(playerDir[1]);
			}
		}
		else if(monsters[i].currState == FIGHT){
			if(playerDistSqr > fightDistSqr){
				monsters[i].currState = SEEK;
			}
			
			monsters[i].timer++;
			
			if(monsters[i].timer >= 60){
				monsters[i].timer = 0;
				(*playerHealth)--;
			}
		}
		else{
			//Shouldn't happen
		}
	}
}