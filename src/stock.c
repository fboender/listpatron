/****************************************************************************
 *
 * ListPatron - stock
 *
 * Stock items for listpatron (icons, etc)
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "stock.h"

static struct stock_icons_ {
	char *stockname;
	char *filename;
} stock_icons[] = {
	{ LP_STOCK_COL_ADD, "listpatron-col-add.png"},
	{ LP_STOCK_COL_DEL, "listpatron-col-del.png"},
	{ LP_STOCK_ROW_ADD, "listpatron-row-add.png"},
	{ LP_STOCK_ROW_DEL, "listpatron-row-del.png"},
	{ NULL, NULL }
};

void load_stock_icons(void) {
	int i;
	GtkIconFactory* ifac_list;
	
	ifac_list = gtk_icon_factory_new();

	i = 0;
	while (stock_icons[i].stockname != NULL) {
		GdkPixbuf *pixb_ico = NULL;
		GError *error = NULL;
		GtkIconSet *icoset_ico = NULL;
		char *filename;
		
		filename = g_build_filename(DATADIR, "pixmaps", "listpatron", "icons", stock_icons[i].filename, NULL);
		pixb_ico = gdk_pixbuf_new_from_file(filename, &error);
		
		if (error != NULL) {
			fprintf(stderr, "Couldn't load icon %s: %s\n", filename, error->message);
			g_error_free(error);
		} else {
			icoset_ico = gtk_icon_set_new_from_pixbuf(pixb_ico);
			gtk_icon_factory_add(ifac_list, stock_icons[i].stockname, icoset_ico);
		}

		g_free(filename);

		i++;
	}

	gtk_icon_factory_add_default(ifac_list);
}
