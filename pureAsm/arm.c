void armFunc1(){
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
	asm("mov r0, r1");
}

void armFunc2(){
	asm("mov r1, r0");
	asm("mov r1, r0");
	asm("mov r1, r0");
	asm("mov r1, r0");
	asm("mov r1, r0");
	asm("mov r1, r0");
	asm("mov r1, r0");
	armFunc1();
}

void armStuff(){
	volatile int x = 6;
	*(&x) = x;
	asm("add r0, r0, r0");
	asm("adds r0, r0, r0");
	asm("add r0, r0, r1");
	asm("add r0, r1, r0");
	asm("add r1, r0, r0");
	asm("add r0, r0, r9");
	asm("add r0, r9, r0");
	asm("add r9, r0, r9");
	asm("add r0, r0, #0");
	asm("add r0, r0, #1");
	asm("add r0, r0, #0xFF");
	asm("add r0, r0, #0x100");
	asm("add r0, r0, #0x100000");
	asm("addeq r0, r0, r0");
	asm("bne .Ltest");
	asm("bne .Ltest");
	asm("bl  .Ltest");
	asm("addne r0, r0, r0");
	// unsure for now
	asm("bne .Ltest");
	asm("add r0, r0, r0, lsl #1");
	asm("bne .Ltest");
	asm(".Ltest:");
	asm("str r0, [r0, #520]");
	asm("str r0, [r0]");
	asm("str r3, [r0]");
	asm("cmp r0, #15");
	asm("bne .Ltest");
	asm("bne .Ltest");
	asm("ldr r0, [r0, #284]");
	asm("ldr r0, [r0]");
	asm("lsl r5, r6, #4");
	asm("mov r0, #0x23");
	asm("mov r3, r6");
	asm("sub r0, r0, #1");
	asm("bne .L4");
	asm("bne .L4");
	asm(".L4:");
	asm("cmp r0, #0");
	asm("bne .L4");
	asm("bne .L4");
	asm("mov r0, #0xFF");
	asm("mov r0, #0x1FC");
	asm("mov r0, #0x3FC");
	asm("mov r0, #0xFF0");
	asm("mov r0, #0xFF00");
	asm("mov r0, #0x3FC0");
	asm("mov r0, #0x3FC00");
	asm("mov r0, #0xFF000");
	asm("mov r0, #0xFF0000");
	asm("mov r0, #0xFF00000");
	asm("swi 0x05");
	
	asm(".LinkL:    ");
	asm("mov r1, r0  ");
	asm("bl .LinkL  ");
	asm("mov r1, r0  ");
	asm("ldr r0, [pc]");
	asm("ldr r0, [pc, #4]");
	asm("ldr r0, [pc, #8]");
	asm("cmp r2, #0");
	
	asm("push {lr}");
	asm("pop {lr}");
	
	armFunc2();
	
}