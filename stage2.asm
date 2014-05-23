[BITS 16]

[GLOBAL _start]
_start:

; Setup GDT at linear 0x10000 (can use up to 256 bytes without affecting
; stage2 code). GDT has 5 entries: NULL, real code, real data, linear code
; and linear data. Linear code and data sections have a base of 0x00000000
; (as the stosw loop below initially zeroes all GDT entries). Real code
; and data sections are byte-granular, 16-bit with a 64kB limit.

GDT_NULL	   equ	0
GDT_RL_CODE	equ	1
GDT_RL_DATA	equ	2
GDT_LN_CODE	equ	3
GDT_LN_DATA	equ	4
GDT_NUM	   equ	5
GDT_RECSZ	equ	8
GDT_BASE	   equ	0x10000

mov	ax, GDT_BASE >> 4
mov	ds, ax
mov	es, ax
xor	di, di
xor	ax, ax
mov	cx, ( GDT_NUM * GDT_RECSZ ) / 2
cld
rep	stosw
mov word	[ GDT_RL_CODE * GDT_RECSZ + 0 ], 0xFFFF	; 1MB/4GB limit
mov word	[ GDT_RL_CODE * GDT_RECSZ + 2 ], 0x0100	; Base 0x10100
mov byte	[ GDT_RL_CODE * GDT_RECSZ + 4 ], 0x01
mov byte	[ GDT_RL_CODE * GDT_RECSZ + 5 ], 0x9A	; Present, ring 0, code, non-conforming & readable
mov word	[ GDT_RL_DATA * GDT_RECSZ + 0 ], 0xFFFF	; 1MB/4GB limit
mov word	[ GDT_RL_DATA * GDT_RECSZ + 2 ], 0x0100	; Base 0x10100
mov byte	[ GDT_RL_CODE * GDT_RECSZ + 4 ], 0x01
mov byte	[ GDT_RL_DATA * GDT_RECSZ + 5 ], 0x92	; Present, ring 0, data, writeable
mov word	[ GDT_LN_CODE * GDT_RECSZ + 0 ], 0xFFFF	; 1MB/4GB limit
mov byte	[ GDT_LN_CODE * GDT_RECSZ + 5 ], 0x9A	; Present, ring 0, code, non-conforming & readable
mov byte	[ GDT_LN_CODE * GDT_RECSZ + 6 ], 0xCF	; Page-granular (4GB limit), 32-bit
mov word	[ GDT_LN_DATA * GDT_RECSZ + 0 ], 0xFFFF	; 1MB/4GB limit
mov byte	[ GDT_LN_DATA * GDT_RECSZ + 5 ], 0x92	; Present, ring 0, data, writeable
mov byte	[ GDT_LN_DATA * GDT_RECSZ + 6 ], 0xCF	; Page-granular (4GB limit), 32-bit
mov word	[ GDT_NUM * GDT_RECSZ + 0 ], GDT_NUM * GDT_RECSZ - 1	
mov dword	[ GDT_NUM * GDT_RECSZ + 2 ], GDT_BASE
lgdt	[ GDT_NUM * GDT_RECSZ + 0 ]

push dword	0				; Zero EFLAGS (interrupts off,
popfd					      ; IOPL = 0, NT bit = 0)

mov	eax, cr0				; Enter protected mode
or	   al, 1
mov	cr0, eax

jmp dword	GDT_LN_CODE * GDT_RECSZ:do_pm	; Switch to 32-bit code

[BITS 32]
do_pm:

mov	eax, GDT_LN_DATA * GDT_RECSZ
mov	ds, eax
mov	es, eax
mov	fs, eax
mov	gs, eax
mov	ss, eax
mov	esp, 0x80000

[EXTERN main]
call main

stall_loop:
hlt
jmp stall_loop

; some useful data
[GLOBAL part_start_lba]
part_start_lba    equ   0x00008118
[GLOBAL part_length]
part_length       equ   0x0000811C
[GLOBAL bios_disk_number]
bios_disk_number  equ   0x00008120
[GLOBAL disk_address_packet]
disk_address_packet  equ   0x00010140

real_mode_segment	      equ	0x1010
real_mode_entry	      equ	0x0100
real_mode_registers	   equ	0x0028
real_mode_sw_int_opcode	equ	0x003A

[GLOBAL real_mode_linear_base]
real_mode_linear_base	equ	real_mode_segment << 4
[GLOBAL real_mode_linear_esp]
real_mode_linear_esp	   equ	real_mode_linear_base + 0x003C

[GLOBAL real_mode_linear_sw_int]
real_mode_linear_sw_int  equ   real_mode_linear_base + real_mode_registers

; real_mode_sw_int_call: run a SW interrupt in real mode
; Linear data addresses
; 0x10128 Flags, DI, SI, BP, --, BX, DX, CX, AX (18 bytes, -- would be SP but in fact, SP is not popped)
; 0x1013A Interrupt instruction (2 bytes, CDxx, to be copied into code segment and executed blind)
; 0x1013C ESP temporary storage (4 bytes)
[GLOBAL real_mode_sw_int_call]
real_mode_sw_int_call:

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

REAL1	equ	real_mode_entry + ( go_real1 - _start )
jmp	GDT_RL_CODE * GDT_RECSZ:REAL1		; Switch to 16-bit code

[BITS 16]

go_real1:

mov	ax, GDT_RL_DATA * GDT_RECSZ		; Load 16-bit segments
mov	ds, ax
mov	es, ax
mov	fs, ax
mov	gs, ax
mov	ss, ax

mov	eax, cr0				; Enter real mode
and	al, 0xFE
mov	cr0, eax

REAL2 equ   real_mode_entry + ( go_real2 - _start )
jmp	real_mode_segment:REAL2		; Need it!!!

go_real2:

mov	ax, cs				; Need these too!!!
mov	ds, ax
mov	es, ax
mov	fs, ax
mov	gs, ax
mov	ss, ax

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Now in real mode
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

mov	ax, [real_mode_sw_int_opcode]		; Setup interrupt instruction

YYY   equ   real_mode_entry + ( software_interrupt - _start )

mov	[YYY], ax

mov	sp, real_mode_registers			; Point to setup structure
popf					; Setup flags (should have been set by caller)
popa					; Setup registers (also set by caller)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Kludge below, beware! Best to have a more sensible stack allocation
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

add   sp, 0xF000
software_interrupt:
dw	0x9090         ; Blindly execute instruction at 0x003A
                  ; 0x90 = NOP, just in case
sub   sp, 0xF000

pusha					; Save registers (to return to caller)
pushf					; Save flags

push dword	0x00000000				; Zero EFLAGS (interrupts off,
popfd					; IOPL = 0, NT bit = 0)

mov	eax, cr0				; Enter protected mode
or	   al, 0x01
mov	cr0, eax

jmp dword	GDT_LN_CODE * GDT_RECSZ:go_protected		; Switch to 32-bit code

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
