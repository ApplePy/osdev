#!/bin/bash
nasm -o bootsect.bin bootsect.asm
nasm -o realmode.com realmode.asm
#nasm -o stage3.bin stage3.s
nasm -f aout -o mm.o mm.asm
nasm -f aout -o stage2.o stage2.asm
nasm -f aout -o enableA20.o enableA20.s
nasm -f aout -o lib16.o lib16.s
nasm -f aout -o lib32.o lib32.s
gcc -mregparm=1 -c -O0 -Wall -o stage2_main.o stage2_main.c
gcc -mregparm=1 -c -O0 -Wall -o stage3.o stage3.c
ld -s -o mm.com --oformat binary -Ttext 0x100 mm.o
ld -s -o stage2.com --oformat binary -Ttext 0x10200 stage2.o stage2_main.o
ld -s -o stage3.bin --oformat binary -Ttext 0x20000 lib32.o stage3.o

echo -n "s1020" > stage2.txt
./fillprep < stage2.com >> stage2.txt
echo "" >> stage2.txt
echo -n "g" >> stage2.txt

echo -n "s3010" > realmode.txt
./fillprep < realmode.com >> realmode.txt
echo "" >> realmode.txt