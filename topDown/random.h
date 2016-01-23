static int randomSeed = 0;

int GetRandom(){
	long rnd = randomSeed;
	rnd = rnd * 961,748,941;
	rnd = rnd + 15,485,867;
	rnd = rnd % 776,531,401;
	randomSeed = (int)rnd;

	return randomSeed;	
}