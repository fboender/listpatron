DESTDIR=$(prefix)  # Debian controlled
BIN = /usr/bin/
CC = gcc
LIBS = `pkg-config --libs gtk+-2.0` `xml2-config --libs`
INCS = `pkg-config --cflags gtk+-2.0` `xml2-config --cflags`
FLAGS = -Wall -c -g

main: listpatron

listpatron: ui_rulelist.o ui_sort.o list.o debug.o libgtkext.o listpatron.o ui_import.o ui_export.o ui_find.o
	$(CC) -Wall -g libgtkext.o debug.o listpatron.o libxmlext.o list.o ui_rulelist.o ui_sort.o ui_import.o ui_export.o ui_find.o $(LIBS) $(INCS) -o listpatron 

listpatron.o: listpatron.c listpatron.h splash.h debug.h list.h splash.h libgtkext.h ui_sort.h ui_import.h ui_export.h ui_find.h
	$(CC) $(FLAGS) $(INCS) listpatron.c

# User interface
ui_rulelist.o: ui_rulelist.c ui_rulelist.h list.h libgtkext.h menu_def.h
	$(CC) $(FLAGS) $(INCS) ui_rulelist.c

ui_sort.o: ui_sort.c ui_sort.h list.h libgtkext.h
	$(CC) $(FLAGS) $(INCS) ui_sort.c

ui_import.o: ui_import.c ui_import.h list.h
	$(CC) $(FLAGS) $(INCS) ui_import.c
	
ui_export.o: ui_export.c ui_export.h list.h
	$(CC) $(FLAGS) $(INCS) ui_export.c

ui_find.o: ui_find.c ui_find.h list.h
	$(CC) $(FLAGS) $(INCS) ui_find.c

# Libraries
list.o: libxmlext.o list.c list.h debug.h listpatron.h libxmlext.h
	$(CC) $(FLAGS) $(INCS) list.c

debug.o: debug.c debug.h
	$(CC) $(FLAGS) $(INCS) debug.c

libxmlext.o: libxmlext.c libxmlext.h
	$(CC) $(FLAGS) $(INCS) libxmlext.c

libgtkext.o: libgtkext.c libgtkext.h
	$(CC) $(FLAGS) $(INCS) libgtkext.c


clean:
	-rm listpatron
	-rm *.o
install:
	echo "Under construction..."
