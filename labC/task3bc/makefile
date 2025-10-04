# Variables
FILE_NAME = myshell
PARSER = LineParser

# All Targets
all: $(FILE_NAME)

# Tool invocations
$(FILE_NAME): $(FILE_NAME).o $(PARSER).o
	gcc -g -m32 -Wall -o $(FILE_NAME) $(FILE_NAME).o $(PARSER).o

$(FILE_NAME).o: $(FILE_NAME).c $(PARSER).h
	gcc -m32 -g -Wall -c -o $(FILE_NAME).o $(FILE_NAME).c

$(PARSER).o: $(PARSER).c $(PARSER).h
	gcc -m32 -g -Wall -c -o $(PARSER).o $(PARSER).c

.PHONY: clean

# Clean the build directory
clean:
	rm -f *.o $(FILE_NAME)

# gcc invoke GNU Compiler
# -g flag for debugging in gdb
# -m32 to tell it it's 32-bit code
# -Wall allows common warning messages
# -o name of output file
# -c compile the file, don't link it
# -o desired output file name