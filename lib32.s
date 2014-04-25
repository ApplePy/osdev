[SECTION .text]
[BITS 32]

[GLOBAL _start]
[EXTERN main]
_start:
	mov byte [es:dword 0xB891e],'G'
	mov byte [es:dword 0xB891f],0x7c
call main
hlt

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
	mov byte [es:dword 0xB8922],'M'
	mov byte [es:dword 0xB8923],0x7c
	push ebp
	mov  ebp, esp
	push ebx
	push edi

	mov  ebx, [ebp+8] ; Get second parameter from stack
;	mov  ebx, 0x2e58
	mov  edi, eax
	mov  byte [es:edi], bl

	pop  edi
	pop  ebx
	pop  ebp
	mov byte [es:dword 0xB8924],'N'
	mov byte [es:dword 0xB8925],0x7c
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

[GLOBAL stackdump]
stackdump:

mov	byte [es:dword 0xB8926],'O'
mov	byte [es:dword 0xB8927],0x7c

push	ebp
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
hlt                     ; SHOULD PRINT 8 DIGITS ABOVE, BUT DO THEY GO SOMEWHERE ELSE?

add	edi, 144
dec	edx
;jnz	stackdumploop1

stackdumphalt:
hlt
jmp	stackdumphalt

[SECTION .data]
