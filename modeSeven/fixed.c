#ifndef FIXED_DECIMAL
#define FIXED_DECIMAL 8
#endif

#define FIXED_ONE (1 << FIXED_DECIMAL)
typedef int int32;
typedef int32 fixed;

static inline fixed makeFixed(int x){
	return x << FIXED_DECIMAL;
}

static inline int32 truncFixedToInt(fixed x){
	return x >> FIXED_DECIMAL;
}

static inline int32 roundFixedToInt(fixed x){
	return (x + (1 << (FIXED_DECIMAL - 1))) >> FIXED_DECIMAL;
}

#define fixedFromFlt(x) ((fixed)((x) * FIXED_ONE))

static const fixed deg2rad = fixedFromFlt(3.1415926536f/180);

static inline fixed fixMult(fixed a, fixed b){
	return (a * b) >> FIXED_DECIMAL;
}

static inline fixed fixDiv(fixed a, fixed b){
	return ((a/b) << FIXED_DECIMAL) + ((a%b) << FIXED_DECIMAL) / b;
}

static inline fixed fixPow(fixed b, int e){
	fixed answer = FIXED_ONE;
	for(int i = 0; i < e; i++){
		answer = fixMult(answer, b);
	}
	
	return answer;
}

static inline fixed macLaurin(fixed in, int deg) {
	fixed val = FIXED_ONE;

	for (int i = 1; i <= deg; i++) {
		val = fixMult(val, in);
		val /= i;
	}

	return val;
}

static inline fixed mySin(fixed in){
	in = in + makeFixed(180);
	if (in > makeFixed(360))
	{
		in = in % makeFixed(360);
	}
	in = in - makeFixed(180);

	const fixed highCutoff = makeFixed(160);
	
	// I linearly interpolate it at the extremes, because error is high.
	// I'm so sorry.
	int isHigh = (in > highCutoff);
	fixed highRatio = fixDiv(in - highCutoff, makeFixed(180) - highCutoff);
	
	int isLow = (in < -highCutoff);
	fixed lowRatio = fixDiv(-in - highCutoff, makeFixed(180) - highCutoff);
	
	in = fixMult(in, deg2rad);

	fixed val = in;
	val -= macLaurin(in, 3);
	val += macLaurin(in, 5);
	val -= macLaurin(in, 7);
	val += macLaurin(in, 9);
	
	if (isHigh){
		val = fixMult(val, (FIXED_ONE - highRatio));
	}
	else if(isLow){
		val = fixMult(val, (FIXED_ONE - lowRatio));
	}
	
	return val;
}

static inline fixed myCos(fixed in){
	in = in + makeFixed(180);
	if (in > makeFixed(360))
	{
		in = in % makeFixed(360);
	}
	in = in - makeFixed(180);
	
	const fixed highCutoff = makeFixed(160);
	int isHigh = (in > highCutoff);
	int isLow = (in < -highCutoff);
	fixed ratio = fixDiv((isLow ? -in : in) - highCutoff, makeFixed(180) - highCutoff);
	
	in = fixMult(in, deg2rad);

	fixed val = FIXED_ONE;
	val -= macLaurin(in, 2);
	val += macLaurin(in, 4);
	val -= macLaurin(in, 6);
	val += macLaurin(in, 8);

	if (isHigh || isLow){
		val = fixMult(val, (FIXED_ONE - ratio)) - ratio;
	}
	
	return val;
}


#if defined(FIXED_TEST_MAIN)

#include <stdio.h>
#include <stdlib.h>

#define MY_ABS(x) (((x) >= 0) ? (x) : -(x))

#define ASSERT(cond) if(!cond){printf("Assertion failed: '%s'\nFile: '%s'\nLine: '%d'\n", #cond, __FILE__, __LINE__);exit(-1);}
#define ASSERT_MSG(cond, msg, ...) if(!cond){printf("Assertion failed: '%s'\nMessage: '" msg "'\nFile: '%s'\nLine: '%d'\n", #cond, __VA_ARGS__, __FILE__, __LINE__);exit(-1);}

void assertEqual(fixed a, fixed b, const char* cond, const char* file, int line){
	if (MY_ABS(a - b) > fixedFromFlt(0.02f)){
		printf("Assertion failed: '%s'\nFile: '%s'\nLine: %d\n", cond, file, line);
		printf("Values: '%f' and '%f'\n", ((float)a)/FIXED_ONE, ((float)b)/FIXED_ONE);
	}
}

#define ASSERT_EQUAL(a, b) assertEqual(a, b, #a "==" #b, __FILE__, __LINE__)

int main(int argc, char** argv){

	for (float x = -170; x < 170.0f; x += 10) {
		printf("myCos(%f): %f  mySin(%f): %f\n", x, ((float)myCos(fixedFromFlt(x))) / FIXED_ONE, x, ((float)mySin(fixedFromFlt(x))) / FIXED_ONE);
	}

	return 0;
}

#endif
