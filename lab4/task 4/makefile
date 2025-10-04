all : task4 task1
task4: task4.o
	gcc -m32 -g -Wall -o task4 task4.o
task4.o: task4.c
	gcc -m32 -g -Wall -fno-pie -fno-stack-protector -m32 -c -o task4.o task4.c
task1: task1.o
	gcc -m32 -g -Wall -o task1 task1.o
task1.o: task1.c
	gcc -m32 -g -Wall -c -o task1.o task1.c

.PHONY: clean

clean:
	rm task4 task4.o
	rm task1 task1.o