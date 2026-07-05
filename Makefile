CC = gcc
# Enable POSIX/GNU functions like nanosleep()
CFLAGS = -Wall -g -D_DEFAULT_SOURCE

OBJ = main.o map.o game.o file.o

escape: $(OBJ)
	$(CC) $(CFLAGS) -o escape $(OBJ)

main.o: main.c escape.h
	$(CC) $(CFLAGS) -c main.c

map.o: map.c escape.h
	$(CC) $(CFLAGS) -c map.c

game.o: game.c escape.h
	$(CC) $(CFLAGS) -c game.c

file.o: file.c escape.h
	$(CC) $(CFLAGS) -c file.c

clean:
	rm -f escape $(OBJ)
