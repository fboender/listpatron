DESTDIR=$(prefix)  # Debian controlled
BIN = /usr/bin/
CC = gcc
LIBS = `pkg-config --libs gtk+-2.0` `xml2-config --libs`
INCS = `pkg-config --cflags gtk+-2.0` `xml2-config --cflags`
FLAGS = -Wall -c -g

#main: listpatron
#	$(CC) -Wall -g listpatron.o libxmlext.o list.o $(LIBS) $(INCS) -o listpatron 
#listpatron: list listpatron.o listpatron.c splash.h
#	$(CC) $(FLAGS) $(INCS) listpatron.c
#list: libxmlext list.o list.c list.h
#	$(CC) $(FLAGS) $(INCS) list.c
#libxmlext: libxmlext.o libxmlext.c libxmlext.h
#	$(CC) $(FLAGS) $(INCS) libxmlext.c

main: listpatron
listpatron: list.o debug.o libgtkext.o listpatron.o 
	$(CC) -Wall -g libgtkext.o debug.o listpatron.o libxmlext.o list.o $(LIBS) $(INCS) -o listpatron 
listpatron.o: listpatron.c listpatron.h splash.h
	$(CC) $(FLAGS) $(INCS) listpatron.c
list.o: libxmlext.o list.c list.h
	$(CC) $(FLAGS) $(INCS) list.c
libxmlext.o: libxmlext.c libxmlext.h
	$(CC) $(FLAGS) $(INCS) libxmlext.c
debug.o: debug.c debug.h
	$(CC) $(FLAGS) $(INCS) debug.c
libgtkext.o: libgtkext.c libgtkext.h
	$(CC) $(FLAGS) $(INCS) libgtkext.c


clean:
	-rm listpatron
	-rm *.o
install:
	echo "Under construction..."
