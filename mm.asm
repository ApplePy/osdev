; mm.s: mini-monitor program for x86

[BITS 16]

TTY	equ 0x03F8

; Start-up code
; mov cx,di is temporary and will eventually be removed

[GLOBAL _start]
_start:
cli
mov	ax,cs
mov	ds,ax

; Initialize serial device (console)

mov	bx,0
mov	cx,TTYINITCOUNT

init_loop:
mov	al,[bx+ttyinitdata]
rol	bx,1
mov	dx,[bx+ttyinitport]
ror	bx,1
inc	bx
out	dx,al
loop	init_loop

; Display start up message

mov	di,info1
mov	bx,startupmsg
jmp	puts

; Information

info1:
mov	di,info2
mov	bx,bootdate
jmp	puts

info2:
mov	ax,0xFFFF
mov	es,ax
mov	bp,5
mov	di,info3

info3:
cmp	bp,15
jz	info4
mov	ah,[es:bp]
inc	bp
cmp	ah,0x20
jb	info4
cmp	ah,0x7F
jae	info4
jmp	putc

info4:
mov	di,info5
mov	bx,codeseg
jmp	puts

info5:
mov	di,prompt
mov	ah,4
mov	bx,cs
jmp	puthex

prompt:
mov	word [echo],0xFFFF	; Turn-on echo
mov	di,req
mov	bx,prompttext
jmp	puts

; Get user request

req:
mov	di,req_test
jmp	getc

req_test:
cmp	ah,'?'
jz	help
or	ah,0x20
cmp	ah,'d'
jz	req_44
cmp	ah,'f'
jz	req_40
cmp	ah,'g'
jz	go
cmp	ah,'i'
jz	req_40
cmp	ah,'j'
jz	req_40
cmp	ah,'o'
jz	req_42
cmp	ah,'m'
jz	req_44
mov	word [echo],0x0000	; Turn-off echo
cmp	ah,'s'
jz	req_40
jmp	prompt

; Help screen

help:
mov	bp,addontbl
mov	di,help_rtn1
mov	bx,helptext
jmp	puts

help_rtn1:
mov	cx,[bp]
cmp	cx,0xFFFF
jz	prompt
add	bp,2
mov	di,help_rtn2
mov	bx,stub1
jmp	puts

help_rtn2:
mov	di,help_rtn3
mov	ah,4
mov	bx,cx
jmp	puthex

help_rtn3:
mov	di,help_rtn4
mov	bx,stub2
jmp	puts

help_rtn4:
mov	di,help_rtn1
mov	bx,bp
add	bp,30
jmp	puts

req_40:
mov	es,ax
mov	di,req_40b
mov	cx,4
jmp	gethex

req_40b:
mov	ax,bx
mov	cx,es
cmp	ch,'i'
jz	req_i
cmp	ch,'j'
jz	req_j

; Memory fill (f or s!)

req_fs:
mov	es,ax
mov	bp,0

req_fs_nl:
mov	di,req_fs_rtn1
mov	ah,0x0A
jmp	putc

req_fs_rtn1:
mov	di,req_fs_rtn2
mov	bx,es
mov	ah,4
jmp	puthex

req_fs_rtn2:
mov	di,req_fs_rtn3
mov	ah,0x3A
jmp	putc

req_fs_rtn3:
mov	di,req_fs_rtn4
mov	bx,bp
mov	ah,4
jmp	puthex

req_fs_rtn4:
mov	di,req_fs_rtn5
mov	cx,2
jmp	gethex

req_fs_rtn5:
mov	[es:bp],bl
inc	bp
test	bp,0x000F
jnz	req_fs_rtn4
jmp	req_fs_nl

; Input from port

req_i:
mov	dx,ax
in	al,dx
mov	bh,al
mov	di,req_i_rtn
mov	ah,0x0A
jmp	putc

req_i_rtn:
mov	di,prompt
mov	ah,2
jmp	puthex

; Intra-segment jump

req_j:
mov	di,req_j_rtn
mov	ah,0x0A
jmp	putc

req_j_rtn:
mov	di,prompt
jmp	bx

req_42:
mov	es,ax
mov	di,req_42b
mov	cx,4
jmp	gethex

req_42b:
mov	bp,bx
mov	di,req_42c
mov	cx,2
jmp	gethex

req_42c:
mov	ax,bp
mov	cx,es

; Output to port

req_o:
mov	dx,ax
mov	ax,bx
out	dx,al
jmp	prompt

req_44:
mov	es,ax
mov	di,req_44b
mov	cx,4
jmp	gethex

req_44b:
mov	bp,bx
mov	di,req_44c
mov	cx,4
jmp	gethex

req_44c:
mov	ax,bp
mov	cx,es
cmp	ch,0x6D
jz	req_m

; Dump range of memory

req_d:
mov	cl,4
ror	bx,cl
test	bh,0xF0
jz	req_d_noinc
and	bh,0x0F
inc	bx

req_d_noinc:
mov	es,ax
mov	bp,bx
mov	si,0
mov	cl,0

req_d_nl:
test	bp,bp
jz	req_d_checksum
mov	di,req_d_rtn1
mov	ah,0x0A
jmp	putc

req_d_rtn1:
mov	di,req_d_rtn2
mov	bx,es
mov	ah,4
jmp	puthex

req_d_rtn2:
mov	di,req_d_rtn3
mov	ah,0x3A
jmp	putc

req_d_rtn3:
mov	di,req_d_rtn4
mov	bx,si
mov	ah,4
jmp	puthex

req_d_rtn4:
mov	di,req_d_rtn5
mov	ah,0x20
jmp	putc

req_d_rtn5:
mov	bh,[es:si]
inc	si
add	cl,bh
mov	di,req_d_rtn6
mov	ah,2
jmp	puthex

req_d_rtn6:
test	si,0x000F
jnz	req_d_rtn4
sub	si,0x10
mov	di,req_d_rtn7
mov	ah,0x20
jmp	putc

req_d_rtn7:
mov	di,req_d_rtn8

req_d_cloop:
mov	ah,[es:si]
inc	si
cmp	ah,0x20
jb	req_d_badc
cmp	ah,0x7F
jae	req_d_badc
jmp	req_d_okc

req_d_badc:
mov	ah,0x2E

req_d_okc:
jmp	putc

req_d_rtn8:
test	si,0x000F
jnz	req_d_cloop
dec	bp
jmp	req_d_nl

req_d_checksum:
mov	di,req_d_rtn9
mov	bx,checksum
jmp	puts

req_d_rtn9:
mov	di,prompt
mov	bh,cl
mov	ah,2
jmp	puthex

; Modify memory location

req_m:
mov	di,req_m_rtn1
mov	ah,0x0A
jmp	putc

req_m_rtn1:
mov	ds,bp
mov	ah,[bx]
mov	cx,cs
mov	ds,cx
mov	es,bx
mov	di,req_m_rtn2
mov	bh,ah
mov	ah,2
jmp	puthex

req_m_rtn2:
mov	di,req_m_rtn3
mov	cx,2
jmp	gethex

req_m_rtn3:
mov	ah,bl
mov	ds,bp
mov	bx,es
mov	[bx],ah
mov	ax,cs
mov	ds,ax
jmp	prompt

; Go! (jmp 0x1010:0x0100)

go:
jmp	0x1010:0x0100

; puthex: Prints a number of hex digits to the console
;
; DI = Return address
; AH = Number of digits
; BX = Value to display, "left justified"
; Alters: AX, BX and DX
puthex:
cmp	word [echo],0
jz	puthex_no_echo
mov	dx,TTY+LSR

puthex_wait:
in	al,dx
test	al,0x20
jz	puthex_wait
mov	al,cl
mov	cl,4
rol	bx,cl
mov	cl,al
mov	dx,bx
and	bx,0x000F
mov	al,[bx+hextable]
mov	bx,dx
mov	dx,TTY
out	dx,al
dec	ah
jnz	puthex

puthex_no_echo:
jmp	di

; puts: Sends a null-terminated string to the console
;
; DI = Return address
; BX = Pointer to string
; Alters: AX, BX, DX, DI and SI
puts:
mov	si,di

puts_loop:
mov	ah,[bx]
test	ah,ah
jz	puts_exit
inc	bx
mov	di,puts_loop
jmp	putc

puts_exit:
jmp	si

; putc: Sends one character to the console
;
; DI = Return address
; AH = Character
; Alters: AX and DX
putc:
cmp	word [echo],0
jz	putc_no_echo
cmp	ah,0x0A
jnz	putc_noendl
mov	ah,0x0D
mov	dx,TTY+LSR

putc_wait1:
in	al,dx
test	al,0x20
jz	putc_wait1
mov	al,ah
mov	dx,TTY
out	dx,al
mov	ah,0x0A

putc_noendl:
mov	dx,TTY+LSR

putc_wait2:
in	al,dx
test	al,0x20
jz	putc_wait2
mov	al,ah
mov	dx,TTY
out	dx,al

putc_no_echo:
jmp	di

; gethex: retrieves a hexadecimal number from the console
; if invalid input is detected, the prompt is displayed again
;
; DI = Return address
; BX = Value retrieved, "right justified"
; CX = Number of digits
; Alters: AX, BX, CX, DX, DI and SI
gethex:
mov	si,di

mov	di,gethex_loop
mov	ah,0x20
jmp	putc

gethex_loop:
mov	di,gethex_rtn
jmp	getc

gethex_rtn:
test	ah,0x40
jz	gethex_num
and	ah,0x5F
cmp	ah,0x41
jb	prompt
cmp	ah,0x46
ja	prompt
sub	ah,0x37
jmp	gethex_gen

gethex_num:
sub	ah,0x30
cmp	ah,9
ja	prompt

gethex_gen:
mov	di,cx
mov	cl,4
shl	bx,cl
mov	cx,di
mov	al,ah
mov	ah,0
or	bx,ax
loop	gethex_loop

jmp	si

; getc: retrieves one character from the console
;       if echo is non-zero, the character is echoed
;
; DI = Return address
; AH = Character retrieved
; Alters: AX and DX
getc:
mov	dx,TTY+LSR

getc_wait1:
in	al,dx
test	al,0x01
jz	getc_wait1
mov	dx,TTY
in	al,dx
mov	ah,al
cmp	word [echo],0
jz	getc_no_echo
mov	dx,TTY+LSR

getc_wait2:
in	al,dx
test	al,0x20
jz	getc_wait2
mov	al,ah
mov	dx,TTY
out	dx,al

getc_no_echo:
jmp	di

testmsg:
mov	bx,testmsgtext
jmp	puts

int0:
mov	ax,0
div	ax

DLLB	equ 0
IER		equ 1
DLHB	equ 1
IIR		equ 2
FCR		equ 2
LCR		equ 3
MCR		equ 4
LSR		equ 5
MSR		equ 6
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

echo:
DW	1

startupmsg:
DB `\nx86 ROM Mini-monitor`
DB `\nCopyright (C) 2001`
DB `\nEnter "?" for help`, 0x00

bootdate:
DB `\nBoot ROM date = `, 0x00

codeseg:
DB `\nDS = CS = `, 0x00

prompttext:
DB `\n>`, 0x00

checksum:
DB `\nChecksum = `, 0x00

hextable:
DB `0123456789ABCDEF`

testmsgtext:
DB `Testing add-on command functionality. If you`
DB `\ncan read this, add-on commands are working.`, 0x00

helptext:
DB `\nInternal commands:`
DB `\nd <seg> <count>  - dump range of memory`
DB `\nf <seg>          - fill range of memory`
DB `\ng                - go! (jmp 0x1010:0x0100)`
DB `\ni <port>         - read byte from I/O port`
DB `\nj <offset>       - intra-segment jump`
DB `\nm <seg> <offset> - modify memory location`
DB `\no <port> <byte>  - write byte to I/O port`
DB `\ns <seg>          - "silent" fill (same as f without echo)`
DB `\nAdd-on commands:`, 0x00

stub1:
DB `\nj `, 0x00

stub2:
DB `           - `, 0x00

addontbl:
DW	testmsg
DB `print test message           `, 0x00
DW	int0
DB `int 0 (divide by zero)       `, 0x00
DW	0xFFFF
