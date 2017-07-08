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
	
	mov sp 0x03000000
	add sp sp 0x7F00
	add sp sp 0xA0
	
	mov r0 31
	@word 0xE129F000 ; This is the same as  'msr CPSR_fc, r0' TODO
	
	; Init user mode stack
	mov sp 0x03000000
	add sp sp 0x7F00
	
	;sub sp sp 4
	
	bl :CpyArmToRam
	
	mov r0 0x5000000
	add r0 r0 4
	
	sub pc pc r0
	
	bl :SetupInterrupts
	
	; Set up bitmap mode
	; REG_DISPLAY = 0x1000 | 0x0040 | 0x0100
	mov r0 0x04000000
	mov r1 0x1140
	str r1 [r0]
	
	; Very bad way of doing ldr r1=0x19FF8CD2
	; Really, we just want variety, since this is our colour value
	;bl :stuff
	
	; Set up some palette colours...like 4?
	mov r0 0x05000000
	add r0 r0 0x200
	mov r1 0x00
	add r1 r1 0xFF0000
	add r1 r1 0xFF000000
	str r1 [r0]
	add r0 r0 4
	mov r1 0xEE
	add r1 r1 0x3800
	add r1 r1 0x190000
	add r1 r1 0x91000000
	str r1 [r0]
	
	; Get a sprite image there
	; VRAM + 512 * 8 * 32 * 4 is where OAM tile memory starts
	mov r0 0x06000000
	add r0 r0 0x10000
	add r0 r0 0x20
	mov r1 1
	add r1 r1 0x10000
	str r1 [r0]
	str r1 [r0 4]
	str r1 [r0 8]
	mov r1 2
	add r1 r1 0x30000
	str r1 [r0 12]
	str r1 [r0 16]
	str r1 [r0 20]
	mov r1 1
	add r1 r1 0x30000
	str r1 [r0 24]
	str r1 [r0 28]
	
	; Set up an oam object
	mov r0 0x07000000
	mov r1 0x00
	add r1 r1 0x370000
	str r1 [r0]
	mov r1 0x01
	str r1 [r0 4]
	
	
	mov r2 30
	
	:MainLoop:
	
	mov r0 0x07000000
	mov r1 r2
	add r1 r1 0x370000
	str r1 [r0]
	
	add r2 r2 1
	
	cmp r2 60 ;; BUG: Is this being assembled as E3520000 (cmp r2 0)?
	b.le :Normal_Loop
	mov r2 30
	:Normal_Loop:
	
	;; TODO: Get interrupts working
	swi 0x05
	
	b :MainLoop
	
@endproc

@proc stuff
	
	ldr r0 [pc 4]
	bx lr
	
	@word 0xFF2200FF
	@word 0xFFFFFFFF
@endproc

; r0 holds color to draw
;;@proc drawColToScreen
;;	mov r1 0x06000000
;;	mov r2 19200
;;	
;;	:Loop0:
;;	str r0 [r1]
;;	sub r2 r2 1
;;	add r1 r1 4
;;	cmp r2 0 ;e3520000
;;	b.eq :Done
;;	b :Loop0
;;	
;;	:Done:
;;	bx lr
;;@endproc

;; Interrupt addrs
;;#define INT_VECTOR	*(IntFn *)(0x03007ffc)
;;#define BNS_REG_IE  (*(volatile uint16*)0x04000200)
;;#define BNS_REG_IF  (*(volatile uint16*)0x04000202)
;;#define BNS_REG_IME (*(volatile uint16*)0x04000208)
;;#define REG_DISPSTAT (*((volatile uint16 *)(MEM_IO + 0x0004)))
;;#define LCDC_VBL (1 << 3)
;;#define IRQ_VBLANK (1 << 0)

;; Do:
;; INT_VECTOR = InterruptMain
;; BNS_REG_IME	= 0;
;; REG_DISPSTAT |= LCDC_VBL;
;; BNS_REG_IE |= IRQ_VBLANK;
;;BNS_REG_IME	= 1;

@proc SetupInterrupts
	
	; Get INT_VECTOR addr into r0
	mov r0 0x03000000
	add r0 r0 0x7F00
	add r0 r0 0xFC
	
	; Store InterruptMain in INT_VECTOR
	ldr r1 [pc 76]
	add r1 r1 0x03000000
	str r1 [r0]
	
	;; BNS_REG_IME = 0
	mov r0 0x04000000
	add r0 r0 0x208
	mov r1 0
	strh r1 [r0]
	
	;; REG_DISPSTAT |= LCDC_VBL
	mov r0 0x04000000
	ldrh r1 [r0 4]
	orr r1 r1 8
	strh r1 [r0 4]
	
	;; BNS_REG_IE |= IRQ_VBLANK
	mov r0 0x04000000
	add r0 r0 0x200
	ldrh r1 [r0]
	orr r1 r1 1
	strh r1 [r0]
	
	;; BNS_REG_IME = 1
	mov r0 0x04000000
	add r0 r0 0x208
	mov r1 1
	strh r1 [r0]

	bx lr
	@labelWord InterruptMainStart
@endproc

@label InterruptMainStart
@proc InterruptMain

	mov	r3 0x4000000
	ldr	r2 [r3 512]	; 0x200
	ldr	r1 [r3 520]	; 0x208
	mov	r0 r1
	and r1 r2 r2 << 16
	ldrh	r2 [r3 -8]  ;;@mix up with BIOS irq flags at 3007FF8h
	orr	r2 r2 r1
	strh	r2 [r3 -8]
	add	r3 r3 512	; 0x200
	strh	r1 [r3 2]
	str	r0 [r3 520]	; 0x208
	mov	pc lr
	bx	lr

@endproc

;; Padding or something I guess?
@proc DummyCodeLol

	mov r0 r0
	mov r1 r1
	mov r2 r2
	
	bx lr

@endproc                                                       


@label ArmCodeEnd