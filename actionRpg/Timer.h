typedef struct{
	int id;
	int frameCount;
	int limit;
} Timer;

//Provide a good default, but can be overridden.
#ifndef MAX_TIMER_COUNT
#define MAX_TIMER_COUNT 50
#endif

typedef struct{
	Timer timers[MAX_TIMER_COUNT];
	int timerCount;
	int currentId;
} TimerList;

void UpdateTimers(TimerList* timerList){
	for(int i = 0; i < timerList->timerCount; i++){
		timerList->timers[i].frameCount++;
		
		if(timerList->timers[i].frameCount >= timerList->timers[i].limit){
			timerList->timers[i] = timerList->timers[timerList->timerCount-1];
			timerList->timerCount--;
			i--;
		}
	}
}

int AddTimer(TimerList* timerList, int timeLimit){
	timerList->timers[timerList->timerCount].id = timerList->currentId;
	timerList->timers[timerList->timerCount].frameCount = 0;
	timerList->timers[timerList->timerCount].limit = timeLimit;
	
	int newId = timerList->timers[timerList->timerCount].id;
	
	timerList->timerCount++;
	timerList->currentId++;
	
	return newId;
}

int IsTimerDone(TimerList* timerList, int id){
	for(int i = 0; i < timerList->timerCount; i++){
		if(timerList->timers[i].id == id){
			return false;
		}
	}
	
	return true;
}