;/****************************************************************************
;       file load.asm
;****************************************************************************/
[BITS 16]

TTY	equ 0x03F8

[GLOBAL _start]
_start:

mov ax, 0xB800
mov es, ax
mov byte [es:0x0910],'H'
mov byte [es:0x0911],0x7c
mov byte [es:0x0912],'e'
mov byte [es:0x0913],0x7c
mov byte [es:0x0914],'l'
mov byte [es:0x0915],0x7c
mov byte [es:0x0916],'l'
mov byte [es:0x0917],0x7c
mov byte [es:0x0918],'o'
mov byte [es:0x0919],0x7c
mov byte [es:0x091a],' '
mov byte [es:0x091b],0x7c




; Setup Device Address Packet (DAP)
;sub	sp, 0x10
;mov	si, sp
;mov word	[si+0x00], 0x0010	; Reserved byte + 16-byte packet size
;mov word	[si+0x02], 0x0001	; Reserved byte + 1 block to transfer
;mov word	[si+0x04], 0x1000	; Offset of transfer buffer
;mov word	[si+0x06], 0x4000	; Segment of transfer buffer
;mov word	[si+0x08], 0x5EB1	; Logical block address (low word)
;mov word	[si+0x0A], 0x02ED	; Logical block address (high word)
;mov word	[si+0x0C], 0x0000       ; Logical block address (MSB)
;mov word	[si+0x0E], 0x0000       ; Restrict logical block address to 4TB

; Send request to BIOS
;mov	ah, 0x42                    ; Extended read
;mov	dl, 0x80
;int	0x13




mov byte [es:0x091c],'W'
mov byte [es:0x091d],0x7c
mov byte [es:0x091e],'o'
mov byte [es:0x091f],0x7c
mov byte [es:0x0920],'r'
mov byte [es:0x0921],0x7c
mov byte [es:0x0922],'l'
mov byte [es:0x0923],0x7c
mov byte [es:0x0924],'d'
mov byte [es:0x0925],0x7c
mov byte [es:0x0926],'!'
mov byte [es:0x0927],0x7c

; Setup GDT at linear 0x10000 (can use up to 256 bytes without affecting
; stage2 code). GDT has 5 entries: NULL, real code, real data, linear code
; and linear data. Linear code and data sections have a base of 0x00000000
; (as the stosw loop below initially zeroes all GDT entries). Real code
; and data sections are byte-granular, 16-bit with a 64kB limit.

GDT_NULL	equ	0
GDT_RL_CODE	equ	1
GDT_RL_DATA	equ	2
GDT_LN_CODE	equ	3
GDT_LN_DATA	equ	4
GDT_NUM	equ	5
GDT_RECSZ	equ	8
GDT_BASE	equ	0x10000

mov	ax, GDT_BASE >> 4
mov	ds, ax
mov	es, ax
xor	di, di
xor	ax, ax
mov	cx, ( GDT_NUM * GDT_RECSZ ) / 2
cld
rep	stosw
mov word	[ GDT_RL_CODE * GDT_RECSZ + 0 ], 0xFFFF	; 1MB/4GB limit
mov word	[ GDT_RL_CODE * GDT_RECSZ + 2 ], 0x0000	; Base 0x30000
mov byte	[ GDT_RL_CODE * GDT_RECSZ + 4 ], 0x03
mov byte	[ GDT_RL_CODE * GDT_RECSZ + 5 ], 0x9A	; Present, ring 0, code, non-conforming & readable
mov word	[ GDT_RL_DATA * GDT_RECSZ + 0 ], 0xFFFF	; 1MB/4GB limit
mov word	[ GDT_RL_DATA * GDT_RECSZ + 2 ], 0x0000	; Base 0x30000
mov byte	[ GDT_RL_CODE * GDT_RECSZ + 4 ], 0x03
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
popfd					; IOPL = 0, NT bit = 0)

mov	eax, cr0				; Enter protected mode
or	al, 1
mov	cr0, eax

STAGE2PM	equ	0x10200 + ( do_pm - _start )	; Switch to 32-bit code
jmp dword	GDT_LN_CODE * GDT_RECSZ:STAGE2PM

[BITS 32]
do_pm:

mov	eax, GDT_LN_DATA * GDT_RECSZ
mov	ds, eax
mov	es, eax
mov	fs, eax
mov	gs, eax
mov	ss, eax
mov	esp, 0x80000

mov byte	[es:dword 0xB891A],'F'
mov byte	[es:dword 0xB891B],0x7c

mov	al, '^'
call putcc

mov byte [es:dword 0xB891c],'@'
mov byte [es:dword 0xB891d],0x7c

;call	0x00020200

[EXTERN main]
call main

hlt

; putcc: Sends one character to the console
;
; AL = Character
; Alters: AX

[GLOBAL putcc]
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

; getcc: retrieves one character from the console
;        if echo is non-zero, the character is echoed
;
; AL = Character retrieved
; Alters: AX

[GLOBAL getcc]
getcc:

push	edx

mov	dx, TTY+LSR

getcc_wait1:
in	al, dx
test	al, 0x01
jz	getcc_wait1
mov	dx, TTY
in	al, dx
mov	ah, al
mov	dx, TTY+LSR

getcc_wait2:
in	al, dx
test	al, 0x20
jz	getcc_wait2
mov	al, ah
mov	dx, TTY
out	dx, al

pop	edx

ret

[GLOBAL reset]
reset:
jmp dword	0x0000:0x00000000






;jmp	dword LINEAR_CODE_SEL:0x20000

mov	byte [es:dword 0xB8926],'O'
mov	byte [es:dword 0xB8927],0x7c

mov     eax, 0x12123434
push    eax
mov     eax, 0x56567878

;push	ebp
mov	ebp, esp

mov	byte [es:dword 0xB8928],'P'
mov	byte [es:dword 0xB8929],0x7c

push	eax
mov	eax, esp
push	eax
mov	eax, 0x89ABCDEF
push	eax
mov	eax, 0x01234567
push	eax

mov	eax, [ebp+8] ; Get second parameter from stack
push	eax

mov	edi, 0xB8000
cld
mov	edx, 16

stackdumploop1:
pop	ebx
mov	ecx, 0x08

stackdumploop2:
rol	ebx, 4
mov	al, bl
and	al, 0x0F
cmp	al, 10
jb	stackdumpnohex
add	al, 7
stackdumpnohex:
add	al, 0x30
stosb
mov	al, 0x4C
stosb
loop	stackdumploop2

add	edi, 144
dec	edx
jnz	stackdumploop1

stackdumphalt:
hlt
jmp	stackdumphalt


[GLOBAL peekb]
peekb:
	push edi

	mov byte [es:dword 0xB891e],'8'
	mov byte [es:dword 0xB891f],0x7c

	mov  edi, eax
	mov  byte al, [es:edi]
	and  eax, 0x000000FF

	pop  edi
	ret

[GLOBAL pokeb]
pokeb:
	mov byte [es:dword 0xB891a],'6'
	mov byte [es:dword 0xB891b],0x7c
	push ebp
	mov  ebp, esp
	push ebx
	push edi

	mov  ebx, [bp+8] ; Get second parameter from stack
	mov  edi, eax
	mov  byte [es:edi], bl

	pop  edi
	pop  ebx
	pop  ebp
	mov byte [es:dword 0xB891c],'7'
	mov byte [es:dword 0xB891d],0x7c
	ret

[GLOBAL my_print]
my_print:
	mov byte [es:dword 0xB8916],'4'
	mov byte [es:dword 0xB8917],0x7c

	push  edi
	push  esi
	push  eax

	mov   esi, eax
	mov   edi, 0x000B8A50
	cld
my_print_loop:
	lodsb
	cmp   al, 0x00
	jz    my_print_exit
	stosb
	mov   al, 0x59
	stosb
	jmp   my_print_loop

my_print_exit:
	pop   eax
	pop   esi
	pop   edi

	mov byte [es:dword 0xB8918],'5'
	mov byte [es:dword 0xB8919],0x7c
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

TTYINITCOUNT	equ 7

ttyinitdata:
DB	0x00	; Disable interrupts
DB	0x00
DB	0x80	; Set baud and framing parameters
DB	0x01	; 115200
;DB	0x0C	; 9600
;DB	0x30	; 2400
DB	0x00
DB	0x03	; 8N1
DB	0xE7	; Enable FIFOs (if any)

ttyinitport:
DW	TTY+LCR
DW	TTY+IER
DW	TTY+LCR
DW	TTY+DLLB
DW	TTY+DLHB
DW	TTY+LCR
DW	TTY+FCR
