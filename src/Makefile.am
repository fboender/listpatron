AUTOMAKE_OPTIONS = gnu
LDADD = @GTK_LIBS@ @XML_LIBS@
CPPFLAGS = \
	-g \
	@GTK_CFLAGS@ \
	@XML_CPPFLAGS@ \
	-DDATADIR=\"$(datadir)\"

bin_PROGRAMS = listpatron
listpatron_SOURCES = debug.c libgtkext.c libxmlext.c list.c listpatron.c \
                     stock.c ui_export.c ui_find.c ui_import.c ui_rulelist.c \
					 ui_sort.c

noinst_HEADERS = debug.h libgtkext.h libxmlext.h list.h listpatron.h \
                 stock.h ui_export.h ui_find.h ui_import.h ui_rulelist.h \
				 ui_sort.h menu_def.h splash.h

# man page
#man_MANS = gdu.1x

# we want these in the dist tarball
#EXTRA_DIST = back.xpm reload.xpm root.xpm \
#             home.xpm stop.xpm about.xpm gdu.1x
