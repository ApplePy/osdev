[BITS 32]

; Might just as well combine memcpy and memmove, since memmove supplies
; the needed functionality for memcpy.
;
;void   *memcpy   ( void *s1,       const void *s2, size_t n );
[GLOBAL memcpy]
memcpy:
;void   *memmove  ( void *s1,       const void *s2, size_t n );
[GLOBAL memmove]
memmove:

push	ebp
mov	ebp, esp

push	edi
push	esi
push	ecx

mov	edi, eax
mov	esi, [ ebp + 8 ]	; 2nd parameter
mov	ecx, [ ebp + 12 ]	; 3rd parameter

cld

cmp	edi, esi
jc	memmove_movsb

std
add	edi, ecx
dec	edi
add	esi, ecx
dec	esi

memmove_movsb:
rep	movsb

pop	ecx
pop	esi
pop	edi

pop	ebp

ret









;int    memcmp    ( const void *s1, const void *s2, size_t n ); // Priority
[GLOBAL memcmp]
memcmp:

push	ebp
mov	ebp, esp

push	edi
push	esi
push	ecx

mov	edi, [ ebp + 8 ]	; 2nd parameter
mov	esi, eax
mov	ecx, [ ebp + 12 ]	; 3rd parameter


cld

repe	cmpsb

dec	edi
dec	esi

xor	eax, eax
lodsb
sub	al, [ es:edi ]
movsx	eax, al

pop	ecx
pop	esi
pop	edi

pop	ebp

ret


;int    strcmp    ( const char *s1, const char *s2           ); // Priority
[GLOBAL strcmp]
strcmp:

push	ebp
mov	ebp, esp

push	edi
push	esi
push	ecx

mov	edi, eax
mov	esi, [ ebp + 8 ]	; 2nd parameter
mov	ecx, [ ebp + 12 ]	; 3rd parameter



cld

repe	cmpsb

pop	ecx
pop	esi
pop	edi

pop	ebp

ret


;int    strncmp   ( const char *s1, const char *s2, size_t n ); // Priority
[GLOBAL strncmp]
strncmp:

push	ebp
mov	ebp, esp

push	edi
push	esi
push	ecx

mov	edi, eax
mov	esi, [ ebp + 8 ]	; 2nd parameter
mov	ecx, [ ebp + 12 ]	; 3rd parameter



cld

repe	cmpsb

pop	ecx
pop	esi
pop	edi

pop	ebp

ret






;void   *memset   ( void *s,        int c,          size_t n );
[GLOBAL memset]
memset:

push	ebp
mov	ebp, esp

push	edi
push	ecx

mov	edi, eax
mov	eax, [ ebp + 8 ]	; 2nd parameter
mov	ecx, [ ebp + 12 ]	; 3rd parameter

cld

rep	stosb

pop	ecx
pop	edi

pop	ebp

ret

;size_t strlen    ( const char *s                            );
[GLOBAL strlen]
strlen:

push	edi
push	ecx

mov	edi, eax
xor	eax, eax
mov	ecx, eax
dec	ecx

cld

repne	scasb

dec	eax
dec	eax
sub	eax, ecx

pop	ecx
pop	edi

ret

