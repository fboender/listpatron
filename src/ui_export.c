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

#include "ui_export.h"
#include "list.h"

extern GtkWidget *win_main;
extern list_ *list;

void ui_export_ps_portrait_cb(GtkWidget *radio, export_ *export) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		export->orientation = ORIENT_PORTRAIT;
	}
}

void ui_export_ps_landscape_cb(GtkWidget *radio, export_ *export) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		export->orientation = ORIENT_LANDSCAPE;
	}
}

void ui_export_delimiter_comma_cb(GtkWidget *radio, export_ *export) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		export->delimiter = ',';
	}
}

void ui_export_delimiter_tab_cb(GtkWidget *radio, export_ *export) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		export->delimiter = '\t';
	}
}

//void ui_menu_file_export_csv_cb(void) {
void ui_export_csv(void) {
	GtkWidget *dia_file_export;
	GtkWidget *vbox;
	GtkWidget *radio_comma, *radio_tab;
	export_ *export;

	export = malloc(sizeof(export_));
	export->delimiter = ',';

	dia_file_export = gtk_file_chooser_dialog_new(
			"Export Character Separated file",
			GTK_WINDOW(win_main),
			GTK_FILE_CHOOSER_ACTION_SAVE, 
			NULL);
	gtk_dialog_add_buttons(
			GTK_DIALOG(dia_file_export),
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			NULL);
	
	/* Build options widget */
	radio_comma = gtk_radio_button_new_with_mnemonic(NULL, "_Comma");
	radio_tab = gtk_radio_button_new_with_mnemonic_from_widget(
			GTK_RADIO_BUTTON(radio_comma),
			"_Tab");
	g_signal_connect(
			radio_comma, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_export_delimiter_comma_cb),
			export);
	g_signal_connect(
			radio_tab, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_export_delimiter_tab_cb),
			export);
			
	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), radio_comma, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), radio_tab, FALSE, FALSE, 3);

	gtk_widget_show_all(vbox);

	/* Prepare import dialog and show */
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dia_file_export), vbox);

	if (gtk_dialog_run(GTK_DIALOG(dia_file_export)) == GTK_RESPONSE_ACCEPT) {
		export->filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_export)));
		list_export_csv(list, export->filename, export->delimiter);
		free(export->filename);
	}

	gtk_widget_destroy(dia_file_export);

	free(export);
}

void ui_export_ps(void) {
	GtkWidget *dia_file_export;
	GtkWidget *vbox;
	GtkWidget *radio_landscape, *radio_portrait;
	export_ *export;

	export = malloc(sizeof(export_));
	export->orientation = ORIENT_PORTRAIT;

	dia_file_export = gtk_file_chooser_dialog_new(
			"Export PostScript file",
			GTK_WINDOW(win_main),
			GTK_FILE_CHOOSER_ACTION_SAVE, 
			NULL);
	gtk_dialog_add_buttons(
			GTK_DIALOG(dia_file_export),
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			NULL);
	
	/* Build options widget */
	radio_portrait = gtk_radio_button_new_with_mnemonic(NULL, "_Portrait");
	radio_landscape = gtk_radio_button_new_with_mnemonic_from_widget(
			GTK_RADIO_BUTTON(radio_portrait),
			"_Landscape");
	g_signal_connect(
			radio_portrait, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_export_ps_portrait_cb),
			export);
	g_signal_connect(
			radio_landscape, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_export_ps_landscape_cb),
			export);
			
	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), radio_portrait, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), radio_landscape, FALSE, FALSE, 3);

	gtk_widget_show_all(vbox);

	/* Prepare import dialog and show */
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dia_file_export), vbox);

	if (gtk_dialog_run(GTK_DIALOG(dia_file_export)) == GTK_RESPONSE_ACCEPT) {
		export->filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_export)));
		list_export_ps(list, export->filename, export->orientation);
		free(export->filename);
	}

	gtk_widget_destroy(dia_file_export);

	free(export);
}

void ui_export_html(void) {
	GtkWidget *dia_file_export;
	char *filename;

	dia_file_export = gtk_file_chooser_dialog_new(
			"Export HTML file",
			GTK_WINDOW(win_main),
			GTK_FILE_CHOOSER_ACTION_SAVE, 
			NULL);
	gtk_dialog_add_buttons(
			GTK_DIALOG(dia_file_export),
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			NULL);
	
	if (gtk_dialog_run(GTK_DIALOG(dia_file_export)) == GTK_RESPONSE_ACCEPT) {
		filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_export)));
		list_export_html(list, filename);
		free(filename);
	}

	gtk_widget_destroy(dia_file_export);
}
