[BITS 32]

[GLOBAL _start]
_start:

[EXTERN main]
call main

stall_loop:
hlt
jmp stall_loop
