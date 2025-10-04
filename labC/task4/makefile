# Variables
FILE_NAME = myshell
PARSER = LineParser
PIPELINE = mypipeline

# All Targets
all: $(FILE_NAME) $(PIPELINE)

# Tool invocations
$(FILE_NAME): $(FILE_NAME).o $(PARSER).o
	gcc -g -m32 -Wall -o $(FILE_NAME) $(FILE_NAME).o $(PARSER).o

$(PIPELINE): $(PIPELINE).o
	gcc -g -m32 -Wall -o $(PIPELINE) $(PIPELINE).o

$(FILE_NAME).o: $(FILE_NAME).c $(PARSER).h
	gcc -m32 -g -Wall -c -o $(FILE_NAME).o $(FILE_NAME).c

$(PARSER).o: $(PARSER).c $(PARSER).h
	gcc -m32 -g -Wall -c -o $(PARSER).o $(PARSER).c

$(PIPELINE).o: $(PIPELINE).c
	gcc -g -m32 -Wall -c -o $(PIPELINE).o $(PIPELINE).c

.PHONY: clean

# Clean the build directory
clean:
	rm -f *.o $(FILE_NAME) $(PIPELINE)
