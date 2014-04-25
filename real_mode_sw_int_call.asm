%include	"real_mode_sw_int.asm"

[GLOBAL real_mode_sw_int_call]
real_mode_sw_int_call:

push	ebp
mov	ebp, esp

push	esi
push	edi
push	ecx

mov	esi, eax
mov	edi, real_mode_linear_sw_int
mov	ecx, 10
cld
rep	movsw

call	real_mode_linear_entry

mov	eax, [ ebp + 8 ]	; 2nd parameter
test	eax, eax
jz	real_mode_sw_int_call_null_ret_pointer

mov	esi, real_mode_linear_sw_int
mov	edi, eax
mov	ecx, 10
cld
rep	movsw

real_mode_sw_int_call_null_ret_pointer:

pop	ecx
pop	edi
pop	esi

pop	ebp

ret

