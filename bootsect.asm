;
; BOOTSECT.S: MiniOS32 Bootloader v2.0 -> Stage 1
;
;             This program has 1 purpose in life; it resides on the volume
;             boot record (VBR) of a hard disk partition and is responsible
;             for loading the MiniOS32 Bootloader -> Stage 2 into RAM and
;             transferring control over to Stage 2.
;
;             The disk and partition on which the bootloader resides is
;             passed from the master boot record (MBR). The bootloader is
;             installed on the 1st sector of the partition; the following
;             sectors are assumed to contain Stage 2. These sectors are
;             loaded into memory beginning at address 0x1010:0x0100,
;             segment registers are initialized, and control is transferred
;             to Stage 2 at 0x1010:0x0100. Note that loading Stage 2 at
;             0x1010:0x0100 means that sector 128 (at an offset of exactly
;             65536 bytes into the booted partition, after the VBR and
;             Stage 2 code) will be located at 0x2000:0x0000; this may
;             prove to be convenient for a possible protected mode Stage 3.
;
;             The number of sectors to be loaded is read from the word
;             immediately preceding the 0x55, 0xAA signature at the end
;             of the VBR. This is designed to allow adjusting the number
;             of sectors to read for Stage 2 without re-assembling the
;             source code.
;
;             As implemented, the MiniOS32 Bootloader v2.0 can load sectors
;             up to 4TB, limited only by the MBR format; it does not
;             understand GPT format to enable support for loading Stage 2
;             from beyond the 4TB limit imposed by MBR format.
;
; Registers:  DL - boot device (BIOS disk number) from MBR
;             DS:SI - partition table entry from MBR
;
;             The boot device and partition table entry are passed to Stage 2
;             memory space, in the 256 bytes before the entry point of
;             0x1010:0x0100:
;
; Address:    0x0000 - JMP instruction (5 bytes) to return control to the
;                      bootloader if Stage 2 returns, which it should not.
;             0x0010 - Partition table entry (16 bytes).
;             0x0020 - Boot device (1 byte).
;
[BITS 16]

ORG 0x7C00

entry:

; Memory address to load and run Stage 2.
%define STAGE2_SEG 0x0810 
%define STAGE2_ENT 0x0100

; Display greeting
mov  ax, str_greeting
call prints

;
; Check if EDD extensions are present (int 13h/41h).
;

mov  bx, 0x55AA
mov  ah, 0x41
int  0x13
jc   edd_err    ; Check EDD extensions present
cmp  bx, 0xAA55 ; Confirming above
jnz  edd_err
test cx, 0x0001 ; Check for fixed disk access subset
jz   edd_err
cmp  ah, 0x30   ; Check for version = 0x30
jnz  edd_err

;
; Load Stage 2 from disk (int 13h/42h).
;
; AX: scratch
; BX: segment to store sector in RAM
; CX: # of sectors to go
; DL: drive #
; BP: LBA of disk sector (high word)
; SI: points to device address packet (DAP)
; DI: LBA of disk sector (low word)
;

; Initialize BP:DI with 32-bit partition start
; DS:SI points to partition table entry.

mov  di, [si+8]                  ; Partition start low word
mov  bp, [si+10]                 ; Partition start high word

push si
sub  sp, 0x10
mov  si, sp

; BIOS disk number to load from.
xor  ax, ax                      ; AX will be used to store MSB of logical
push ax                          ; block address to enable access to drives
mov  al, dl                      ; up to 4TB; without this approach, access
call printh                      ; would be limited to the first 2TB, with
                                 ; an undesireable "wrap-around" to the start
                                 ; of the disk for LBA > 2TB.

; Sectors to read
mov  ax, bp
call printh
mov  ax, di
call printh

; Number of sectors to read; change the word just before the 55AA signature
; to change the number of sectors loaded without re-assembly.
mov  cx, [num_sect]
mov  ax, cx
call printh

; Destination for the blocks in memory.
mov  bx, STAGE2_SEG
mov  ax, bx
call printh
mov  ax, STAGE2_ENT
call printh

call printcrlf

read_loop:

; Setup Device Address Packet (DAP)
mov  word [si+0x00], 0x0010      ; Reserved byte + 16-byte packet size
mov  word [si+0x02], 0x0001      ; Reserved byte + 1 block to transfer
mov  word [si+0x04], STAGE2_ENT  ; Offset of transfer buffer
mov  word [si+0x06], bx          ; Segment of transfer buffer
add  bx, 0x0020                  ; Post-increment for next 512 bytes of RAM
mov  ax, 0x0D                    ; Carriage return
call printc
pop  ax                          ; Get MSB of logical block address
inc  di                          ; Pre-increment AX:BP:DI; start reading the
jnc  no_carry                    ; sector after the boatloader!
inc  bp
jnc  no_carry
inc  ax
no_carry:
push ax                          ; Save MSB of LBA
mov  word [si+0x08], di          ; Logical block address (low word)
mov  word [si+0x0A], bp          ; Logical block address (high word)
mov  word [si+0x0C], ax          ; Logical block address (MSB)
call printh                      ; Display sector about to be read
mov  ax, bp
call printh
mov  ax, di
call printh
xor  ax, ax
mov  word [si+0x0E], ax          ; Restrict logical block address to 4TB

; Send request to BIOS
mov  ah, 0x42                    ; Extended read
int  0x13
jnc  read_ok
mov  ax, str_disk_read_fail
jmp  error
read_ok:

loop read_loop

call printcrlf

add  sp, 0x12                    ; Save popping AX
pop  si

; Setup environment for Stage 2 w/ partition table entry and boot device.
mov  bx, STAGE2_SEG
mov  es, bx
xor  ax, ax
xor  di, di
cld
mov  cx, 0x0100
rep
stosb
mov  di, 0x0010
cld
mov  cx, di
rep
movsb
mov  ds, bx
mov  [di], dx
mov  word [0xFFFE], 0x0000
mov  byte [0x0000], 0xEA
mov  word [0x0001], return
mov  word [0x0003], cs

; "Sanity check": print data at Stage 2 entry point. No checking is done by
; software, but the data may help in diagnosing a problem loading Stage 2.
;mov  ax, [STAGE2_ENT]
;call printh
;push es
;mov  ax, STAGE2_SEG
;mov  es, bx
mov  ax, [ es : STAGE2_ENT + 0 ]
call printh
mov  ax, [ es : STAGE2_ENT + 2 ]
call printh
;pop  es
call printcrlf

; Setup stack and registers.
mov  ss, bx
mov  sp, 0xFFFE
mov  bp, sp
xor  ax, ax
xor  bx, bx
xor  cx, cx
xor  dx, dx
xor  si, si
xor  di, di

; Transfer control to Stage 2.
jmp  STAGE2_SEG:STAGE2_ENT

; Stage 2 should never return; if it does, handle the error.
return:
mov ax, cs
mov ds, ax
mov ss, ax
mov sp, 0x1FFE
mov ax, str_stage2_return

; Print an error message and halt processing.
error:
push ax
mov  ax, str_error
call prints
pop  ax
call prints
halt:
hlt
jmp halt

; If the EDD detection didn't take.
edd_err:
mov  ax, str_edd_unsupported
jmp  error

;
; printc():  Print a single character to the screen using BIOS int 10h.
;
; void printc( char character );
;

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

printcrlf:
mov  ax, str_crlf
call prints
ret

;
; printh():  Print AX in hexadecimal format, followed by a space.
;
; void printh( unsigned short num );
;

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

; String constants.

str_greeting:
DB  "MiniOS32 Bootloader v2.0 -> Stage 1"

str_crlf:
DB  0x0D, 0x0A, 0x00

str_error:
DB  0x0D, 0x0A, "Error: ", 0x00

str_edd_unsupported:
DB  "int 13h extensions unsupported.", 0x00

str_disk_read_fail:
DB  "disk read failed.", 0x00

str_stage2_return:
DB  "stage 2 returned.", 0x00

; Pad to 508 bytes, then add number of sectors to load and 0x55 0xAA signature

size    EQU     $ - entry
%if size > 512 - 4
  %error "code is too large for boot sector"
%endif
TIMES   (512 - 4 - size)        DB      0x00

num_sect:
DW   0x007F
DB   0x55, 0xAA