[BITS 16]

;
; printc():  Print a single character to the screen using BIOS int 10h.
;
; void printc( char character );

[GLOBAL printc]
printc:

push bx
push ax

mov  ah, 0x0F   ; get current video mode
int  0x10       ; al = video mode, ah = columns per line, bh = active page

pop  ax         ; character to write
mov  ah, 0x0E   ; write character in teletype (TTY) mode
int  0x10

pop  bx

ret

;
; printcrlf():  Print carriage-return followed by line feed.
;
; void printcrlf();
;

[GLOBAL printcrlf]
printcrlf:
mov  ax, str_crlf
call prints
ret

str_crlf:
DB  0x0D, 0x0A, 0x00

;
; printh():  Print AX in hexadecimal format, followed by a space.
;
; void printh( unsigned short num );
;

[GLOBAL printh]
printh:

push bx
push cx

mov  bx, ax
mov  cx, 4

printh_loop:
mov  ax, bx
rol  bx, 4
shr  ax, 12
cmp  ax, 10
jl   printh_skip
add  ax, 7
printh_skip:
add  ax, 48
call printc
loop printh_loop

pop  cx
pop  bx

mov  ax, 0x20
call printc

ret

;
; prints():  Print a string of characters to the screen using printc().
;
; void prints( char *string );
;

[GLOBAL prints]
prints:

push si

mov  si, ax
prints_loop:
cld
lodsb
test al, al
jz   prints_exit
call printc
jmp  prints_loop
prints_exit:

pop  si

ret
