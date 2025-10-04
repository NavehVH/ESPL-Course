all : hexeditplus
hexeditplus: hexeditplus.o
	gcc -m32 -g -Wall -o hexeditplus hexeditplus.o
hexeditplus.o: hexeditplus.c
	gcc -m32 -g -Wall -c -o hexeditplus.o hexeditplus.c

.PHONY: clean

clean:
	rm hexeditplus hexeditplus.o