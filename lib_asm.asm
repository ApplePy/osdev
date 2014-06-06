; get_eip: returns the value of EIP, from more-or-less where the function was called
[GLOBAL get_eip]
get_eip:
pop   eax
push  eax
ret

; halt: halt the CPU
[GLOBAL halt]
halt:
cli
hlt
jmp   halt

; inb: Read byte from input port
[GLOBAL inb]
inb:
push  edx

mov   edx, eax
in    al, dx

pop   edx
ret

; inw: Read word from input port
[GLOBAL inw]
inw:
push  edx

mov   edx, eax
in    ax, dx

pop   edx
ret

; ind: Read double-word from input port
[GLOBAL ind]
ind:
push  edx

mov   edx, eax
in    eax, dx

pop   edx
ret

; irq_disable: mask interrupts
[GLOBAL irq_disable]
irq_disable:
cli
ret

; irq_enable: unmask interrupts
[GLOBAL irq_enable]
irq_enable:
sti
ret

; lgdt: Load GDTR
[GLOBAL lgdt]
lgdt:
push  ebp
mov   ebp, esp

lgdt  [eax]
mov word ax, [ebp+8]
mov word [lgdt_reload_segment_regs-2], ax
jmp   dword 0x0000:lgdt_reload_segment_regs

lgdt_reload_segment_regs:
mov   eax, [ebp+12]
mov   ds, ax
mov   es, ax
mov   ss, ax
mov   fs, ax
mov   gs, ax

pop   ebp
ret

; lidt: Load IDTR
[GLOBAL lidt]
lidt:
lidt  [eax]
ret

; outb: Write byte to output port
[GLOBAL outb]
outb:
push  ebp
mov   ebp, esp
push  edx

mov   edx, eax
mov   al, [ebp+8]
out   dx, al

pop   edx
pop   ebp
ret

; outw: Write word to output port
[GLOBAL outw]
outw:
push  ebp
mov   ebp, esp
push  edx

mov   edx, eax
mov   ax, [ebp+8]
out   dx, ax

pop   edx
pop   ebp
ret

; outd: Write double-word to output port
[GLOBAL outd]
outd:
push  ebp
mov   ebp, esp
push  edx

mov   edx, eax
mov   eax, [ebp+8]
out   dx, eax

pop   edx
pop   ebp
ret

; reset: triple-fault and therefore reset CPU
[GLOBAL reset]
reset:
; prevent CPU from servicing any exceptions
mov   eax, reset_init_idt
call  lidt
; attempt to use descriptor 0, which will cause a fault
jmp   dword 0x0000:0x00000000
hlt
jmp   reset
reset_init_idt:
dw 0x0000
dd 0x00000000

; sgdt: Save GDTR
[GLOBAL sgdt]
sgdt:
sgdt  [eax]
ret

; sidt: Save IDTR
[GLOBAL sidt]
sidt:
sidt  [eax]
ret

[GLOBAL software_interrupt]
software_interrupt:
mov   [software_interrupt+6], al
db    0xCD, 0x00
ret

[GLOBAL ISR_Template_Start]
ISR_Template_Start:

; Push an "error code" in order to maintain consistent stack frame; only
; some exceptions push an extra error code:
;
; 8 - double fault
; 10 - bad TSS
; 11 - segment not present
; 12 - stack fault
; 13 - general protection fault
; 14 - page fault
;
; Modify code (replace with 9090... no-op) for these exceptions, as they
; push the error code on their own.
[GLOBAL ISR_Template_Error_Start]
ISR_Template_Error_Start:
push byte   0x00
[GLOBAL ISR_Template_Error_End]
ISR_Template_Error_End:

; Save registers
pushad
push  ds
push  es
push  fs
push  gs

; Initialize kernel data segments
mov   ax, 0x0010
mov   ds, ax
mov   es, ax
mov   fs, ax
mov   gs, ax

; Set interrupt number
;
; Modify code (replace with interrupt number) for all exceptions.
; Used to increment interrupt count AND passed to C function handler.
[GLOBAL ISR_Template_Num]
ISR_Template_Num           equ   $+1
mov   eax, 0x00000000

; Increment interrupt count
shl   eax, 2
[EXTERN irq_count]
inc dword   [eax+irq_count]
shr   eax, 2

; Call C function handler
;
; Modify code (replace with 9090... no-op) if function pointer is NULL.
[GLOBAL ISR_Template_CFunc_Start]
ISR_Template_CFunc_Start:
;[EXTERN irq_cfunc]
;mov   ebx, [eax+irq_cfunc]
[GLOBAL ISR_Template_CFunc_Addr]
ISR_Template_CFunc_Addr    equ   $+1
mov   ebx, 0x12345678
call  ebx
[GLOBAL ISR_Template_CFunc_End]
ISR_Template_CFunc_End:

; Restore registers
pop   gs
pop   fs
pop   es
pop   ds
popad

add   esp, 4 ; clear-up error code

; End of ISR
iretd

[GLOBAL ISR_Template_End]
ISR_Template_End:
