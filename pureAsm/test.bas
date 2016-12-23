@proc multiply

	@word 0xEA00002E
	@word 0x51AEFF24
	@word 0x21A29A69
	@word 0x0A82843D
	@word 0xAD09E484
	@word 0x988B2411
	@word 0x217F81C0
	@word 0x19BE52A3
	@word 0x20CE0993
	@word 0x4A4A4610
	@word 0xEC3127F8
	@word 0x33E8C758
	@word 0xBFCEE382
	@word 0x94DFF485
	@word 0xC1094BCE
	@word 0xC08A5694
	@word 0xFCA77213
	@word 0x734D849F
	@word 0x619ACAA3
	@word 0x27A39758
	@word 0x769803FC
	@word 0x61C71D23
	@word 0x56AE0403
	@word 0x008438BF
	@word 0xFD0EA740
	@word 0x03FE52FF
	@word 0xF130956F
	@word 0x85C0FB97
	@word 0x2580D660
	@word 0x03BE63A9
	@word 0xE2384E01
	@word 0xFF34A2F9
	@word 0x44033EBB
	@word 0xCB900078
	@word 0x943A1188
	@word 0x637CC065
	@word 0xAF3CF087
	@word 0x8BE425D6
	@word 0x72AC0A38
	@word 0x07F8D421
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000
	@word 0x00963130
	@word 0x00000080
	@word 0x00000000
	@word 0x00007000
	@word 0xEA000006
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000
	@word 0x00000000

	mov r0 0x4000000
	str r0 [r0 520]
	mov r0 18
	@word 0xE129F000 ; This is the same as  'msr CPSR_fc, r0' TODO
	mov r0 31
	@word 0xE129F000 ; This is the same as  'msr CPSR_fc, r0' TODO
	
	;set sp to 03007fa0
	;mov r0 0x03000000
	;add r0 r0 0x7F00
	;add r0 r0 0xa0
	;mov sp r0
	
	;sub sp sp 4
	
	;; InterruptMain proc
	;; mov	r3, #0x4000000		@ REG_BASE                                        
	;; ldr	r2, [r3,#0x200]		@ Read	REG_IE                                    
	;; ldr	r1, [r3, #0x208]	@ r1 = IME                                        
	;; mov	r0, r1		                                                          
	;; and	r1, r2,	r2, lsr #16	@ r1 =	IE & IF                                   
	;; ldrh	r2, [r3, #-8]		 @mix up with BIOS irq flags at 3007FF8h,     
	;; orr	r2, r2, r1		@ aka mirrored at 3FFFFF8h, this is required          
	;; strh	r2, [r3, #-8]		@/when using the (VBlank)IntrWait functions   
	;; add	r3,r3,#0x200                                                          
	;; strh	r1, [r3, #0x02]		@ IF Clear                                    
	;; str	r0, [r3, #0x208]	@ restore REG_IME                                 
	;; mov	pc,lr                                                                 
	
	;INT_VECTOR = InterruptMain;
	;BNS_REG_IME	= 0;
	;REG_DISPSTAT |= LCDC_VBL;
	;BNS_REG_IE |= IRQ_VBLANK;
	;BNS_REG_IME	= 1;
	
	; Set up bitmap mode
	mov r0 0x04000000
	mov r1 0x0003
	add r1 r1 0x0400
	str r1 [r0]
	
	; Very bad way of doing ldr r1=0x19FF8CD2
	; Really, we just want variety, since this is our colour value
	bl :stuff
	
	:Loop0:
	mov r0 0x06000000
	mov r2 1024
	
	:Loop1:
	;@word 0xEF000005 ; swi 0x05 Vblank wait
	add r1 r1 1
	str r1 [r0]
	sub r2 r2 1
	add r0 r0 4
	cmp r2 0
	b.eq :Loop0
	b :Loop1
	
@endproc

@proc stuff
	mov r1 0
	mov r2 0xD2
	add r1 r1 r2
	mov r2 0x8C00
	add r1 r1 r2
	mov r2 0xFF0000
	add r1 r1 r2
	mov r2 0x19000000
	add r1 r1 r2
	
	bx lr
@endproc