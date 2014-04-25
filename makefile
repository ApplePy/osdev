all: bootsect.bin realmode.txt mm.com stage2.txt

bootsect.bin: bootsect.asm
	nasm -o bootsect.bin bootsect.asm

realmode.com: realmode.asm
	nasm -o realmode.com realmode.asm

mm.o: mm.asm
	nasm -f aout -o mm.o mm.asm

stage2.o: stage2.asm
	nasm -f aout -o stage2.o stage2.asm

stage2_main.o: stage2_main.c
	gcc -fno-builtin -mregparm=1 -c -O0 -Wall -o stage2_main.o stage2_main.c

mm.com: mm.o
	ld -s -o mm.com --oformat binary -Ttext 0x100 mm.o

stage2.com: stage2.o stage2_main.o real_mode_sw_int_call.o real_mode_sw_int.o string.o
	ld -s -o stage2.com --oformat binary -Ttext 0x10200 stage2.o stage2_main.o real_mode_sw_int_call.o real_mode_sw_int.o string.o

realmode.txt: realmode.com
	echo -n "s3010" > realmode.txt
	./fillprep < realmode.com >> realmode.txt
	echo "" >> realmode.txt

stage2.txt: stage2.com
	echo -n "s1020" > stage2.txt
	./fillprep < stage2.com >> stage2.txt
	echo "" >> stage2.txt
	echo -n "g" >> stage2.txt

real_mode_sw_int_call.o: real_mode_sw_int_call.asm
	nasm -f aout -o real_mode_sw_int_call.o real_mode_sw_int_call.asm

string.o: string.asm string.h
	nasm -f aout -o string.o string.asm

real_mode_sw_int.o: real_mode_sw_int.c real_mode_sw_int.h
	gcc -mregparm=1 -c -O0 -Wall -o real_mode_sw_int.o real_mode_sw_int.c
