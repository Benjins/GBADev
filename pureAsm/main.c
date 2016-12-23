


int main(){
	asm("add r0, r0, r0");
	// adds is not supported
	//asm("adds r0, r0, r0");
	asm("add r0, r0, r1");
	asm("add r0, r1, r0");
	asm("add r1, r0, r0");
	asm("add r0, r0, r9");
	asm("add r0, r9, r0");
	asm("add r9, r0, r9");
	asm("add r0, r0, #0");
	asm("add r0, r0, #1");
	asm("add r0, r0, #0xFF");
	//asm("add r0, r0, #0x100");
	//asm("add r0, r0, #0x100000");
	// Conditionals not supported in Thumb
	//asm("addeq r0, r0, r0");
	//asm("addne r0, r0, r0");
	
	//unsure for now
	//asm("add r0, r0, r0, lsl #1");
	
	asm("mov r0, #0x23");
	asm("mov r2, r5");


	return 0;
}