CC = gcc
# FOR MEMORY LEAKS: 
CFLAGS = -Wall -Wextra -g -pthread
# FOR RACE CONDITIONS:
# CFLAGS = -Wall -Wextra -g -pthread -fsanitize=thread 

OBJ = main.o house.o hunter.o ghost.o utils.o helpers.o

all: simulation

simulation: $(OBJ)
	$(CC) $(CFLAGS) -o simulation $(OBJ)

main.o: main.c defs.h helpers.h
	$(CC) $(CFLAGS) -c main.c

house.o: house.c defs.h
	$(CC) $(CFLAGS) -c house.c

hunter.o: hunter.c defs.h helpers.h
	$(CC) $(CFLAGS) -c hunter.c

ghost.o: ghost.c defs.h helpers.h
	$(CC) $(CFLAGS) -c ghost.c

utils.o: utils.c defs.h
	$(CC) $(CFLAGS) -c utils.c

helpers.o: helpers.c helpers.h defs.h
	$(CC) $(CFLAGS) -c helpers.c

clean:
	rm -f *.o simulation log_*.csv