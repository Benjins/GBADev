#ifndef FIXED_DECIMAL
#define FIXED_DECIMAL 8
#endif

#define FIXED_ONE (1 << FIXED_DECIMAL)
typedef int32 fixed;

static inline fixed makeFixed(int x){
	return x << FIXED_DECIMAL;
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

static inline fixed mySin(fixed in){
	
	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return in - fixPow(in,3)/6 + fixPow(in,5)/120 - fixPow(in,7)/5040 + fixPow(in,9)/362880;
}

static inline fixed myCos(fixed in){

	in = in + makeFixed(180);
	in = in % makeFixed(360);
	in = in - makeFixed(180);
	
	in = fixMult(in, deg2rad);
	return FIXED_ONE - fixMult(in,in)/4 + fixPow(in,4)/24 - fixPow(in,6)/720 + fixPow(in,8)/40320;
}

static inline fixed mySqrt(fixed in){
	int reduce = (in >= makeFixed(4));
	if(reduce){
		in /= 4;
	}
	
	in -= FIXED_ONE;
	
	fixed guess = FIXED_ONE + in/2 - fixMult(in,in)/8 + fixPow(in,3)/16 - 5*fixPow(in,4)/128 + 7*fixPow(in,5)/256;
	
	in += FIXED_ONE;
	
	for(int i = 0; i < 10; i++){
		if(guess == 0){
			break;
		}
		guess = (guess + fixDiv(in, guess))/2;
	}
	
	if(reduce){
		guess *= 2;
	}
	
	return abs(guess);
}

static inline int roundFixedToInt(fixed x){
	return (x + FIXED_ONE/2) >> FIXED_DECIMAL;
}