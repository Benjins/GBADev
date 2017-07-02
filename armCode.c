void InterruptMain() __attribute__((section(".iwram")));

#define BNS_REG_BASE 0x4000000
#define BNS_REG_IE  0x04000200
#define BNS_REG_IF  0x04000202
#define BNS_REG_IME 0x04000208
	
/*
Lol, workflow.
*/
void InterruptMain() {
asm("	mov	r3, #0x4000000		@ REG_BASE                                                         ");
asm("	ldr	r2, [r3,#0x200]		@ Read	REG_IE                                                     ");
asm("	ldr	r1, [r3, #0x208]	@ r1 = IME                                                         ");
asm("	mov	r0, r1		                                                                           ");
asm("	and	r1, r2,	r2, lsr #16	@ r1 =	IE & IF                                                    ");
asm("	ldrh	r2, [r3, #-8]		 @mix up with BIOS irq flags at 3007FF8h,                      ");
asm("	ldr 	r2, [r3, #-8]		 @mix up with BIOS irq flags at 3007FF8h,                      ");
asm("	ldrh	r0, [r1]          	 @mix up with BIOS irq flags at 3007FF8h,                      ");
asm("	ldr 	r0, [r1]          	 @mix up with BIOS irq flags at 3007FF8h,                      ");
asm("	orr	r2, r2, r1		@ aka mirrored at 3FFFFF8h, this is required                           ");
asm("	strh	r2, [r3, #-8]		@/when using the (VBlank)IntrWait functions                    ");
asm("	add	r3,r3,#0x200                                                                           ");
asm("	strh	r1, [r3, #0x02]		@ IF Clear                                                     ");
asm("	str	r0, [r3, #0x208]	@ restore REG_IME                                                  ");
asm("	mov	pc,lr                                                                                  ");
}