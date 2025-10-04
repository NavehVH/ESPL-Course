# Define the compiler and compiler flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Define the target executable
TARGET = mypipeline

# Default rule to build the target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(TARGET).o
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).o

# Rule to compile the source file into an object file
$(TARGET).o: $(TARGET).c
	$(CC) $(CFLAGS) -c $(TARGET).c

# Rule to run the program
run: $(TARGET)
	./$(TARGET)

# Rule to clean the build files
clean:
	rm -f $(TARGET) $(TARGET).o

# Phony targets
.PHONY: all run clean
