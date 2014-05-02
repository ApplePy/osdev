[BITS 32]
[GLOBAL _start]
_start:
	mov byte [es:dword 0xB8918],'Z'
	mov byte [es:dword 0xB8919],0x7c
ret
