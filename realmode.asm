%include	"real_mode_sw_int.asm"
TTY	equ	0x03F8

[BITS 32]

ORG 	real_mode_entry

_start:

GDT_NULL	equ	0
GDT_RL_CODE	equ	1
GDT_RL_DATA	equ	2
GDT_LN_CODE	equ	3
GDT_LN_DATA	equ	4
GDT_NUM	equ	5
GDT_RECSZ	equ	8
GDT_BASE	equ	0x00010000

; Linear data addresses
; 0x3FFE8 Flags, DI, SI, BP, --, BX, DX, CX, AX (18 bytes, -- would be SP but in fact, SP is not popped)
; 0x3FFFA Interrupt instruction (2 bytes, CDxx, to be copied into code segment and executed blind)
; 0x3FFFC ESP temporary storage (4 bytes)

pushad					; Save registers for later
pushfd					; Save flags for later

mov	[real_mode_linear_esp], esp		; Save ESP so we can ret later!

push dword	0x00000000				; Zero EFLAGS (interrupts off,
popfd					; IOPL = 0, NT bit = 0)

xor	eax, eax				; Clear high words of registers just in case
mov	ebx, eax
mov	ecx, eax
mov	edx, eax
mov	esp, eax
mov	ebp, eax
mov	esi, eax
mov	edi, eax

REAL	equ	real_mode_entry + ( go_real - _start )
jmp	GDT_RL_CODE * GDT_RECSZ:REAL		; Switch to 16-bit code

[BITS 16]

go_real:

mov	ax, GDT_RL_DATA * GDT_RECSZ		; Load 16-bit segments
mov	ds, ax
mov	es, ax
mov	fs, ax
mov	gs, ax
mov	ss, ax

mov	eax, cr0				; Enter real mode
and	al, 0xFE
mov	cr0, eax

jmp	real_mode_segment:go_real_for_real		; Need it!!!

go_real_for_real:

mov	ax, cs				; Need these too!!!
mov	ds, ax
mov	es, ax
mov	fs, ax
mov	gs, ax
mov	ss, ax

mov	ax, [real_mode_sw_int_opcode]		; Setup interrupt instruction
mov	[software_interrupt], ax

mov	sp, real_mode_registers			; Point to setup structure
popf					; Setup flags (should have been set by caller)
popa					; Setup registers (also set by caller)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Now in real mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

software_interrupt:
dw	0x9090				; Blindly execute instruction at 0xFFE8
					; 0x90 = NOP, just in case

pusha					; Save registers (to return to caller)
pushf					; Save flags

push dword	0x00000000				; Zero EFLAGS (interrupts off,
popfd					; IOPL = 0, NT bit = 0)

mov	eax, cr0				; Enter protected mode
or	al, 0x01
mov	cr0, eax

PROTECTED	equ	real_mode_linear_entry + ( go_protected - _start )
jmp dword	GDT_LN_CODE * GDT_RECSZ:PROTECTED		; Switch to 32-bit code

check_i_flag:

push	ax

pushf
pop	ax
and	ax, 0x0200
jnz	i_flag_set
mov	al, 0x0F
i_flag_set:
add	al, 'I'
call	putcc

pop	ax

ret

putcc:

push	edx

mov	ah, al

cmp	ah, 0x0A
jnz	putcc_noendl
mov	ah, 0x0D
mov	dx, TTY+LSR

putcc_wait1:
in	al, dx
test	al, 0x20
jz	putcc_wait1
mov	al, ah
mov	dx, TTY
out	dx, al
mov	ah, 0x0A

putcc_noendl:
mov	dx,TTY+LSR

putcc_wait2:
in	al, dx
test	al, 0x20
jz	putcc_wait2
mov	al, ah
mov	dx, TTY
out	dx, al

pop	edx

ret	

[BITS 32]

go_protected:

mov	eax, GDT_LN_DATA * GDT_RECSZ
mov	ds, eax
mov	es, eax
mov	fs, eax
mov	gs, eax
mov	ss, eax

mov	esp, [real_mode_linear_esp]		; Restore ESP

popfd					; Restore flags
popad					; Restore registers

ret

DLLB	equ 0
IER	equ 1
DLHB	equ 1
IIR	equ 2
FCR	equ 2
LCR	equ 3
MCR	equ 4
LSR	equ 5
MSR	equ 6
SCRATCH	equ 7
