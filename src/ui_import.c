/****************************************************************************
 *
 * ListPatron - ui_import
 *
 * User Interface routines for importing files
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#include <malloc.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>

#include "ui_import.h"
#include "list.h"

extern GtkWidget *win_main;
extern list_ *list;

void ui_import_delimiter_comma_cb(GtkWidget *radio, import_ *import) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		import->delimiter = ',';
	}
}

void ui_import_delimiter_tab_cb(GtkWidget *radio, import_ *import) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		import->delimiter = '\t';
	}
}

void ui_import_csv(void) {
	GtkWidget *dia_file_import;
	GtkWidget *vbox;
	GtkWidget *radio_comma, *radio_tab;
	import_ *import;

	import = malloc(sizeof(import_));
	import->delimiter = ',';

	dia_file_import = gtk_file_chooser_dialog_new(
			"Import character separated file",
			GTK_WINDOW(win_main),
			GTK_FILE_CHOOSER_ACTION_OPEN, 
			NULL);
	gtk_dialog_add_buttons(
			GTK_DIALOG(dia_file_import),
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			NULL);
	
	/* Build options widget */
	radio_comma = gtk_radio_button_new_with_mnemonic(NULL, "_Comma separated");
	radio_tab = gtk_radio_button_new_with_mnemonic_from_widget(
			GTK_RADIO_BUTTON(radio_comma),
			"_Tab separated");
	g_signal_connect(
			radio_comma, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_import_delimiter_comma_cb),
			import);
	g_signal_connect(
			radio_tab, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_import_delimiter_tab_cb),
			import);
			
	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), radio_comma, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), radio_tab, FALSE, FALSE, 3);

	gtk_widget_show_all(vbox);

	/* Prepare import dialog and show */
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dia_file_import), vbox);

	if (gtk_dialog_run(GTK_DIALOG(dia_file_import)) == GTK_RESPONSE_ACCEPT) {
		list_clear();
		list = list_create();
		import->filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_import)));
		list_import_csv(list, import->filename, import->delimiter);
		free(import->filename);
	}

	gtk_widget_destroy(dia_file_import);

	free(import);
}

