all: bootsect.bin mm.com stage2.txt kernel.txt fillprep

bootsect.bin: bootsect.asm
	nasm -o bootsect.bin bootsect.asm

mm.com: mm.o
	ld -s -o mm.com --oformat binary -Ttext 0x100 mm.o

mm.o: mm.asm
	nasm -f aout -o mm.o mm.asm

stage2.txt: stage2.bin fillprep
	echo -n "s1020" > stage2.txt
	./fillprep < stage2.bin >> stage2.txt
	echo "" >> stage2.txt
	echo -n "g" >> stage2.txt

stage2.bin: stage2.o stage2_main.o lib_asm.o lib_c.o string.o
	ld -s -o stage2.bin --oformat binary -Ttext 0x10200 stage2.o stage2_main.o lib_asm.o lib_c.o string.o

stage2.o: stage2.asm
	nasm -f aout -o stage2.o stage2.asm

stage2_main.o: stage2_main.c lib_asm.h lib_c.h string.h
	gcc -fno-builtin -mregparm=1 -c -O0 -Wall -o stage2_main.o stage2_main.c

kernel.txt: kernel.bin fillprep stage2.txt delay.txt
	cat stage2.txt > kernel.txt
	cat delay.txt >> kernel.txt
	echo -n "s00100000" >> kernel.txt
	./fillprep < kernel.bin >> kernel.txt
	echo "" >> kernel.txt
	echo -n "c00100000" >> kernel.txt

kernel.bin: kernel.o kernel_main.o lib_asm.o lib_c.o string.o
	ld -s -o kernel.bin --oformat binary -Ttext 0x100000 kernel.o kernel_main.o lib_asm.o lib_c.o string.o

kernel.o: kernel.asm
	nasm -f aout -o kernel.o kernel.asm

kernel_main.o: kernel_main.c lib_asm.h lib_c.h string.h
	gcc -fno-builtin -mregparm=1 -c -O0 -Wall -o kernel_main.o kernel_main.c

lib_asm.o: lib_asm.asm lib_asm.h
	nasm -f aout -o lib_asm.o lib_asm.asm

lib_c.o: lib_c.c lib_c.h
	gcc -fno-builtin -mregparm=1 -c -O0 -Wall -o lib_c.o lib_c.c

string.o: string.asm
	nasm -f aout -o string.o string.asm

fillprep: fillprep.c
	gcc -o fillprep fillprep.c

clean:
	rm -f *.bin *.o *.com
