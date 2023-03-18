
# compile AUbatch executable
CC = gcc

EXEC = aubatch process

all: $(EXEC)

aubatch:
	@echo "Starting to compile aubatch..."
	$(CC) -g -lm -o aubatch aubatch.c -lpthread -std=c99
	@echo "Finished!"

process:
	@echo "Starting to compile process..."
	$(CC) -g -lm -o process process.c -std=c99
	@echo "Finished!"

# delete old aubatch executable
clean:
	@echo "Removing aubatch executable..."
	rm -f aubatch process
	@echo "Finished!"

