/****************************************************************************
 *
 * ListPatron - ui_find
 *
 * User Interface routines for finding stuff in the list
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>

#include "ui_find.h"
#include "libgtkext.h"
#include "list.h"

extern GtkWidget *win_main;
extern GtkTreeView *treeview;
extern list_ *list;

void ui_find_find_cb(GtkWidget *ent_needle, find_ *find) {
	char *needle = NULL;
	int row, col;
	GtkTreePath *occ_path = NULL;
	GtkTreeViewColumn *occ_col = NULL;

	needle = (char *)gtk_entry_get_text(GTK_ENTRY(ent_needle));

	if (needle) {
		int find_options = 0;

		if (find->matchcase == 1) {
			find_options = find_options | FIND_MATCHCASE;
		}
		if (find->matchfull == 1) {
			find_options = find_options | FIND_MATCHFULL;
		}

		if (list_find(list, needle, find_options, &row, &col)) {
			char *path_str = malloc(sizeof(char) * 10);

			/* int Row -> path */
			sprintf(path_str, "%i", row);
			occ_path = gtk_tree_path_new_from_string(path_str);
			/* int Col -> viewcolumn */
			occ_col = gtk_tree_view_get_column(treeview, col);

			gtk_tree_view_set_cursor(treeview, occ_path, occ_col, 0);
			free (path_str);
		} else {
			gtk_error_dialog("No (more) matches found.");
			gtk_statusbar_msg("No (more) matches found.");
		}
	}
}

void ui_find_toggle_matchcase_cb(GtkWidget *toggle, find_ *find) {
	find->matchcase ^= 1;
}

void ui_find_toggle_matchfull_cb(GtkWidget *toggle, find_ *find) {
	find->matchfull ^= 1;
}

void ui_find(void) {
	GtkWidget *dia_find;
	GtkWidget *vbox;
	gint result;
	find_ *find = malloc(sizeof(find_));
	GtkWidget *toggle_matchcase, *toggle_matchfull;
	
	if (list->liststore == NULL || treeview == NULL) {
		gtk_error_dialog("No data in list yet");
		return;
	}

	find->matchcase = 0;
	find->matchfull = 0;

	dia_find = gtk_dialog_new_with_buttons(
			"Find",
			NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_FIND, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			NULL);

	find->ent_needle = gtk_entry_new();
	g_signal_connect(find->ent_needle, "activate", (GCallback) ui_find_find_cb, find);

	/* Build options widget */
	toggle_matchcase = gtk_check_button_new_with_mnemonic("Case _sensitive");
	toggle_matchfull = gtk_check_button_new_with_mnemonic("F_ull matches only");
	g_signal_connect(
			toggle_matchcase, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_find_toggle_matchcase_cb),
			find);
	g_signal_connect(
			toggle_matchfull, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_find_toggle_matchfull_cb),
			find);
			
	vbox = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), find->ent_needle, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), toggle_matchcase, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(vbox), toggle_matchfull, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_find)->vbox), GTK_WIDGET(vbox), FALSE, TRUE, 0);
	
	gtk_widget_show_all(dia_find);

	while ((result = gtk_dialog_run(GTK_DIALOG(dia_find))) != GTK_RESPONSE_REJECT) {
		ui_find_find_cb(find->ent_needle, find);
	}

	gtk_widget_destroy(dia_find);

}

