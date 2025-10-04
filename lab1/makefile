
# Variables
FILE_NAME = menu_map

# All Targets
all: $(FILE_NAME)

# Tool invocations
$(FILE_NAME): $(FILE_NAME).o
	gcc -g -m32 -Wall -o $(FILE_NAME) $(FILE_NAME).o

$(FILE_NAME).o: $(FILE_NAME).c
	gcc -m32 -g -Wall -c -o $(FILE_NAME).o $(FILE_NAME).c

.PHONY: clean

# Clean the build directory
clean:
	rm -f *.o $(FILE_NAME)

# gcc invoke GNU Compiler
# -g flag for debuffing in gdb
# -m32 to tell it its 32 bit code
# -Wall allows common warning messages
# -0 name of output file
# -c compile the file, dont link it
# -o desired output file name