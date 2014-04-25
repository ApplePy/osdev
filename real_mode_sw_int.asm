[BITS 32]

real_mode_segment	equ	0x3000
real_mode_entry	equ	0x0100
real_mode_registers	equ	0xFFE8
real_mode_sw_int_opcode	equ	0xFFFA

[GLOBAL real_mode_linear_base]
real_mode_linear_base	equ	real_mode_segment << 4
real_mode_linear_esp	equ	real_mode_linear_base + 0xFFFC
real_mode_linear_entry	equ	real_mode_linear_base + real_mode_entry

[GLOBAL real_mode_linear_sw_int]
real_mode_linear_sw_int	equ	real_mode_linear_base + real_mode_registers
