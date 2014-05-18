all: bootsect.bin mm.com stage2.txt stage3.txt fillprep

bootsect.bin: bootsect.asm
	nasm -o bootsect.bin bootsect.asm

mm.com: mm.o
	ld -s -o mm.com --oformat binary -Ttext 0x100 mm.o

mm.o: mm.asm
	nasm -f aout -o mm.o mm.asm

stage2.txt: stage2.com fillprep
	echo -n "s1020" > stage2.txt
	./fillprep < stage2.com >> stage2.txt
	echo "" >> stage2.txt
	echo -n "g" >> stage2.txt

stage2.com: stage2.o stage2_main.o string.o
	ld -s -o stage2.com --oformat binary -Ttext 0x10200 stage2.o stage2_main.o string.o

stage2.o: stage2.asm
	nasm -f aout -o stage2.o stage2.asm

stage2_main.o: stage2_main.c
	gcc -fno-builtin -mregparm=1 -c -O0 -Wall -o stage2_main.o stage2_main.c -std=c99

string.o: string.asm string.h
	nasm -f aout -o string.o string.asm

stage2.txt: stage3.com fillprep
	echo -n "s00200000" > stage3.txt
	./fillprep < stage3.com >> stage3.txt
	echo "" >> stage3.txt

stage3.com: stage3.o
	ld -s -o stage3.com --oformat binary -Ttext 0x200000 stage3.o

stage3.o: stage3.asm
	nasm -f aout -o stage3.o stage3.asm

fillprep: fillprep.c
	gcc -o fillprep fillprep.c -std=c99

clean:
	rm -f *.bin *.o *.com
