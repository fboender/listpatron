DESTDIR=$(prefix)  # Debian controlled
BIN=/usr/bin/
CC = gcc
LIBS = `pkg-config --libs gtk+-2.0`
INCS = `pkg-config --cflags gtk+-2.0`
FLAGS = -Wall -c -g

main: listpatron
	$(CC) -Wall -g listpatron.o $(LIBS) $(INCS) -o listpatron 
listpatron:
	$(CC) $(FLAGS) $(INCS) listpatron.c
clean:
	-rm lk
	-rm *.o
install:
	echo "Under construction..."
