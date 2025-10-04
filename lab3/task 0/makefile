
CC=gcc
ASM=nasm
LD=ld
CFLAGS=-m32 -Wall -ansi -c -nostdlib -fno-stack-protector
LDFLAGS=-m elf_i386
ASMFLAGS=-f elf32

all: task0

start.o: start.s
	$(ASM) $(ASMFLAGS) start.s -o start.o

main.o: main.c util.h
	$(CC) $(CFLAGS) main.c -o main.o

util.o: util.c util.h
	$(CC) $(CFLAGS) util.c -o util.o

task0: start.o main.o util.o
	$(LD) $(LDFLAGS) start.o main.o util.o -o task0

clean:
	rm -f *.o task0