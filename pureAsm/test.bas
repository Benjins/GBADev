@proc rom_start
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
	
@endproc

@proc veryStart
	b :init
@endproc

; r0 dst
; r1 src
; r2 size
@proc CpyData
	:CopyLoop:
	
	cmp r2 0
	b.le :Done
	
	ldr r3 [r1]
	str r3 [r0]
	
	add r0 r0 4
	add r1 r1 4
	sub r2 r2 4
	
	b :CopyLoop
	
	:Done:
	bx lr
@endproc

@proc CpyArmToRam
	@word 0xE52DE004 ; push lr
	
	ldr r1 [pc 24]
	ldr r0 [pc 24]
	sub r2 r0 r1
	add r1 r1 0x08000000
	sub r0 r1 0x05000000
	bl :CpyData
	
	@word 0xE49DE004 ; pop lr
	bx lr
	
	@labelWord ArmCodeStart
	@labelWord ArmCodeEnd
@endproc

@label ArmCodeStart

@proc init

	mov r0 0x4000000
	str r0 [r0 520]
	mov r0 18
	@word 0xE129F000 ; This is the same as  'msr CPSR_fc, r0' TODO
	;; TODO: Set IRQ stack
	mov r0 31
	@word 0xE129F000 ; This is the same as  'msr CPSR_fc, r0' TODO
	
	; Init user mode stack
	mov sp 0x03000000
	add sp sp 0x7F00
	add sp sp 0xA0
	
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
	
	bl :CpyArmToRam
	
	mov r0 0x5000000
	add r0 r0 4
	
	sub pc pc r0
	
	; Set up bitmap mode
	mov r0 0x04000000
	mov r1 0x0003
	add r1 r1 0x0400
	str r1 [r0]
	
	; Very bad way of doing ldr r1=0x19FF8CD2
	; Really, we just want variety, since this is our colour value
	bl :stuff
	
	:MainLoop:
	
	bl :drawColToScreen
	sub r0 r0 1
	sub r0 r0 0x10000
	b :MainLoop
	
@endproc

@proc stuff
	
	ldr r0 [pc 4]
	bx lr
	
	@word 0xFF2200FF
	@word 0xFFFFFFFF
@endproc

; r0 holds color to draw
@proc drawColToScreen
	mov r1 0x06000000
	mov r2 9600
	
	:Loop0:
	str r0 [r1]
	sub r2 r2 1
	add r1 r1 4
	cmp r2 0 ;e3520000
	b.eq :Done
	b :Loop0
	
	:Done:
	bx lr
@endproc

@label ArmCodeEnd