DESTDIR=$(prefix)  # Debian controlled
BIN = /usr/bin/
CC = gcc
LIBS = `pkg-config --libs gtk+-2.0` `xml2-config --libs`
INCS = `pkg-config --cflags gtk+-2.0` `xml2-config --cflags`
FLAGS = -Wall -c -g

main: listpatron
	$(CC) -Wall -g listpatron.o $(LIBS) $(INCS) -o listpatron 
listpatron: listpatron.c splash.h
	$(CC) $(FLAGS) $(INCS) listpatron.c

clean:
	-rm listpatron
	-rm *.o
install:
	echo "Under construction..."
