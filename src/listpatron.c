/****************************************************************************
 *
 * ListPatron
 *
 * A small GTK program for keeping lists of stuff.
 *
 * Author  : Ferry Boender <f DOT boender AT electricmonk DOT nl>
 * License : GPL, General Public License
 * Todo    : - Freeing of all allocated items (GTK)
 *
 * Copyright (C) 2004 Ferry Boender.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 * 
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include <glib-object.h>

#include "debug.h"
#include "listpatron.h"
#include "list.h"
#include "splash.h"
#include "libgtkext.h"

/****************************************************************************
 * Data initializer
 ****************************************************************************/
static GtkItemFactoryEntry ui_menu_items[] = {
	{ "/_File"                            , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/File/_New"                        , NULL , ui_menu_file_new_cb          , 0 , "<StockItem>", GTK_STOCK_NEW    },
	{ "/File/_Open"                       , NULL , ui_menu_file_open_cb         , 0 , "<StockItem>", GTK_STOCK_OPEN   },
	{ "/File/_Save"                       , NULL , ui_menu_file_save_cb         , 0 , "<StockItem>", GTK_STOCK_SAVE   },
	{ "/File/Save _As"                    , NULL , ui_menu_file_save_as_cb      , 0 , "<Item>"                        },
	{ "/File/_Import"                     , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/File/Import/_Character Separated" , NULL , ui_menu_file_import_csv_cb   , 0 , "<Item>"                        },
	{ "/File/_Export"                     , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/File/Export/_Postscript"          , NULL , ui_menu_file_export_ps_cb    , 0 , "<Item>"                        },
	{ "/File/Export/_Html"                , NULL , ui_menu_file_export_html_cb  , 0 , "<Item>"                        },
	{ "/File/sep1"                        , NULL , NULL                         , 0 , "<Separator>"                   },
	{ "/File/_Quit"                       , NULL , ui_menu_file_quit_cb         , 0 , "<StockItem>", GTK_STOCK_QUIT   },
	{ "/_Edit"                            , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/Edit/Cu_t"                        , NULL , NULL                         , 0 , "<StockItem>", GTK_STOCK_CUT    },
	{ "/Edit/_Copy"                       , NULL , NULL                         , 0 , "<StockItem>", GTK_STOCK_COPY   },
	{ "/Edit/_Paste"                      , NULL , NULL                         , 0 , "<StockItem>", GTK_STOCK_PASTE  },
	{ "/Edit/sep1"                        , NULL , NULL                         , 0 , "<Separator>"                   },
	{ "/_Column"                          , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/Column/_Add"                      , NULL , ui_menu_column_add_cb        , 0 , "<StockItem>", GTK_STOCK_ADD    },
	{ "/Column/_Delete"                   , NULL , ui_menu_column_delete_cb     , 0 , "<StockItem>", GTK_STOCK_DELETE },
	{ "/Column/_Rename"                   , NULL , ui_menu_column_rename_cb     , 0 , "<Item>"                        },
	{ "/_Row"                             , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/Row/_Add"                         , NULL , ui_menu_row_add_cb           , 0 , "<StockItem>", GTK_STOCK_ADD    },
	{ "/Row/_Delete"                      , NULL , ui_menu_row_delete_cb        , 0 , "<StockItem>", GTK_STOCK_DELETE },
#ifdef _DEBUG
	{ "/_Debug"                           , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/Debug/_Add test data"             , NULL , ui_menu_debug_addtestdata_cb , 0 , "<Item>"                        },
	{ "/Debug/Add test _rows"             , NULL , ui_menu_debug_addtestrows_cb , 0 , "<Item>"                        },
#endif
	{ "/_Help"                            , NULL , NULL                         , 0 , "<LastBranch>"                  },
	{ "/_Help/About"                      , NULL , ui_menu_help_about_cb        , 0 , "<Item>"                        },
};

static gint ui_nmenu_items = sizeof(ui_menu_items) / sizeof(ui_menu_items[0]);
GtkWidget *win_main;
GtkWidget *lbl_listtitle;
guint sb_context_id;
GtkWidget *sb_status;
GtkTreeView *treeview;

extern list_ *list;

int 
	opt_help,
	opt_batch,
	opt_verbose,
	opt_version;

/****************************************************************************
 * Callbacks 
 ****************************************************************************/
/* Tree and list */
void ui_treeview_cursor_changed_cb(GtkTreeView *tv, gpointer user_data) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	char *path_str;
	int col, row;

	gtk_tree_view_get_cursor(treeview, &path, &column);
	if (path != NULL) {
		path_str = gtk_tree_path_to_string(path);
		gtk_tree_path_free(path);
		row = atoi(path_str);

		col = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "col_nr"));

		gtk_statusbar_msg("Row %i, Column %i", row+1, col+1);

		if (row == list->nr_of_rows-1) {
			list_row_add_empty(list);
		}
	}
}

void ui_menu_file_new_cb(void) {
	list_clear();
	list = list_create();
}

/* File open */
//void ui_file_open_btn_ok_cb(GtkWidget *win, GtkFileSelection *fs) {
//	char *filename = NULL;
//	int err_nr;
//	
//	filename = (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));
//	
//	if ((err_nr = list_load(list, filename)) != 0) {
//		switch (err_nr) {
//			case -1: gtk_error_dialog("Couldn't open file '%s'.", filename); break;
//			case -2: gtk_error_dialog("Invalid listpatron file '%s'.", filename); break;
//			default: gtk_error_dialog("Unknown error while opening file '%s'.", filename); break;	
//		}
//	} else {
//		gtk_statusbar_msg("File '%s' loaded.", filename);
//	}
//}

void ui_menu_file_open_cb(void) {
	GtkWidget *dia_file_open;

	if (list_save_check(list) == -1) {
		return;
	}

	dia_file_open = gtk_file_chooser_dialog_new(
			"Open file",
			GTK_WINDOW(win_main),
			GTK_FILE_CHOOSER_ACTION_OPEN, 
			NULL);
	gtk_dialog_add_buttons(
			GTK_DIALOG(dia_file_open),
			GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			NULL);
	
	if (gtk_dialog_run(GTK_DIALOG(dia_file_open)) == GTK_RESPONSE_ACCEPT) {
		char *filename = NULL;
		int err_nr;
		list_clear();
		list = list_create();

		filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_open)));

		if ((err_nr = list_load(list, filename)) != 0) {
			switch (err_nr) {
				case -1: gtk_error_dialog("Couldn't open file '%s'.", filename); break;
				case -2: gtk_error_dialog("Invalid listpatron file '%s'.", filename); break;
				default: gtk_error_dialog("Unknown error while opening file '%s'.", filename); break;	
			}
		} else {
			gtk_statusbar_msg("File '%s' loaded.", filename);
			if (list->filename != NULL) {
				free(list->filename);
			}
			list->filename = strdup(filename);
		}
		
		free(filename);
	}

	gtk_widget_destroy(dia_file_open);
}

/* File import */
/* Unused */
//void ui_file_import_csv_btn_ok_cb(GtkWidget *win, GtkFileSelection *fs) {
//	char *filename = NULL;
//	char *delimiter_string = NULL;
//	int rows = -1;
//	
//	filename = (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)); /* FIXME: Should this be freed? */
//	
//	delimiter_string = gtk_input_dialog("Enter a single character which delimits the fields in the file", ",");
//	if (delimiter_string != NULL) {
//		if (list_import_csv(list, filename, ',') == -1) {
//			gtk_error_dialog("Not a correct Comma Separated file '%s'", filename);
//		} else {
//			gtk_statusbar_msg("File %s imported. %i rows read.", filename, list->nr_of_rows, rows);
//		}
//
//		free(delimiter_string);
//	}
//}

void ui_file_import_delimiter_comma_cb(GtkWidget *radio, import_ *import) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		import->delimiter = ',';
	}
}

void ui_file_import_delimiter_tab_cb(GtkWidget *radio, import_ *import) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		import->delimiter = '\t';
	}
}

void ui_menu_file_import_csv_cb(void) {
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
	gtk_signal_connect(
			GTK_OBJECT(radio_comma), 
			"toggled",
			GTK_SIGNAL_FUNC(ui_file_import_delimiter_comma_cb),
			import);
	gtk_signal_connect(
			GTK_OBJECT(radio_tab), 
			"toggled",
			GTK_SIGNAL_FUNC(ui_file_import_delimiter_tab_cb),
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

void ui_file_export_ps_portrait_cb(GtkWidget *radio, export_ *export) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		export->orientation = ORIENT_PORTRAIT;
	}
}

void ui_file_export_ps_landscape_cb(GtkWidget *radio, export_ *export) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		export->orientation = ORIENT_LANDSCAPE;
	}
}

void ui_menu_file_export_ps_cb(void) {
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
	gtk_signal_connect(
			GTK_OBJECT(radio_portrait), 
			"toggled",
			GTK_SIGNAL_FUNC(ui_file_export_ps_portrait_cb),
			export);
	gtk_signal_connect(
			GTK_OBJECT(radio_landscape), 
			"toggled",
			GTK_SIGNAL_FUNC(ui_file_export_ps_landscape_cb),
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

void ui_menu_file_export_html_cb(void) {
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

/* File save */
void ui_menu_file_save_cb(void) {
	GtkWidget *dia_file_save;
	int response;

	assert (list->title != NULL);

	if (list->filename == NULL) {
		char *filename;

		if (list->filename != NULL) {
			strdup (list->filename);
		} else {
			filename = malloc(sizeof(char) * (strlen(list->title) + 5));
			sprintf (filename, "%s.lip", list->title);
		}
		
		dia_file_save = gtk_file_chooser_dialog_new(
				"Save",
				GTK_WINDOW(win_main),
				GTK_FILE_CHOOSER_ACTION_SAVE, 
				NULL);
		gtk_dialog_add_buttons(
				GTK_DIALOG(dia_file_save),
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
				NULL);
		gtk_file_chooser_set_current_name (
				GTK_FILE_CHOOSER(dia_file_save),
				filename);

		free (filename);
		
		response = gtk_dialog_run(GTK_DIALOG(dia_file_save));

		if (response == GTK_RESPONSE_ACCEPT) {
			list->filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_save)));
			gtk_widget_destroy(dia_file_save);
		} else {
			gtk_widget_destroy(dia_file_save);
			return;
		}
	}

	if (list->filename != NULL) {
		list_save(list, list->filename);
	}
}

/* File save as... */
void ui_menu_file_save_as_cb(void) {
		GtkWidget *dia_file_save;
	int response;
	char *filename;

	assert (list->title != NULL);


	if (list->filename != NULL) {
		strdup (list->filename);
	} else {
		filename = malloc(sizeof(char) * (strlen(list->title) + 5));
		sprintf (filename, "%s.lip", list->title);
	}
	
	dia_file_save = gtk_file_chooser_dialog_new(
			"Save",
			GTK_WINDOW(win_main),
			GTK_FILE_CHOOSER_ACTION_SAVE, 
			NULL);
	gtk_dialog_add_buttons(
			GTK_DIALOG(dia_file_save),
			GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
			NULL);
	gtk_file_chooser_set_current_name (
			GTK_FILE_CHOOSER(dia_file_save),
			filename);

	free (filename);
	
	response = gtk_dialog_run(GTK_DIALOG(dia_file_save));

	if (response == GTK_RESPONSE_ACCEPT) {
		list->filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_save)));
		gtk_widget_destroy(dia_file_save);
	} else {
		gtk_widget_destroy(dia_file_save);
		return;
	}

	if (list->filename != NULL) {
		list_save(list, list->filename);
	}
}

void ui_menu_file_quit_cb(void) {
	int response;

	response = list_save_check(list);
	if (response != -1) {
		gtk_main_quit();
	}
}

/* Column menu options */
void ui_menu_column_add_cb(void) {
	char *column_name = NULL;
	
	column_name = gtk_input_dialog("Enter the column name", "Col");
	if (column_name) {
		list_column_add(list, column_name);
		free(column_name);
	}
}

void ui_menu_column_rename_cb(void) {
	char *column_name = NULL;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	
	gtk_tree_view_get_cursor(treeview, &path, &column);
	if (path != NULL) {
		gtk_tree_path_free(path);
	}

	if (column == NULL) {
		return;
	}

	column_name = gtk_input_dialog(
			"Enter the column name", 
			(char *)gtk_tree_view_column_get_title(column));

	if (column_name) {
		gtk_tree_view_column_set_title(column, column_name);
		free(column_name);
	}
}

void ui_menu_column_delete_cb(void) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	
	gtk_tree_view_get_cursor(treeview, &path, &column);
	if (path != NULL) {
		gtk_tree_path_free(path);
	}

	list_column_delete(list, column);
}

/* Row menu options */
void ui_menu_row_add_cb(void) {
	list_row_add_empty(list);
}

void gtk_tree_selection_get_references(
		GtkTreeModel *model,
		GtkTreePath *path,
		GtkTreeIter *iter,
		gpointer data) {
	GList **row_refs = data;

	GtkTreeRowReference *row_ref;

	row_ref = gtk_tree_row_reference_new(model, path);
	*row_refs = g_list_append (*row_refs, row_ref);
}

void ui_menu_row_delete_cb(void) {
	GtkTreeSelection *selection = NULL;
	GList *row_refs = NULL;
	GList *iter = NULL;
	
	selection = gtk_tree_view_get_selection(treeview);

	gtk_tree_selection_selected_foreach(selection, gtk_tree_selection_get_references, &row_refs);
	list_row_delete(list, row_refs);

	iter = row_refs;
	while (iter != NULL) {
		gtk_tree_row_reference_free(iter->data);
		iter = iter->next;
	}
	g_list_free(row_refs);
}

/* Debugging menu items */
void ui_menu_debug_addtestdata_cb(void) {
	int col, row;
	char *col_headers[] = {
		"Col A", 
		"Col B", 
		"Col C", 
		"Col D", 
		"Col E"
	};
	char **col_vals;
	int count_start = list->nr_of_rows;
	
	for (col = 0; col < _TEST_COLS; col++) {
		list_column_add(list, col_headers[col]);
	}
	
	for (row = 0; row < _TEST_ROWS; row++) {
		col_vals = malloc(sizeof(void *) * _TEST_COLS);
		
		for (col = 0; col < _TEST_COLS; col++) {
			char *value = NULL;
			
			value = malloc(sizeof(char) * (strlen(col_headers[col] + 2)));
			sprintf(value, "%s-%02i", col_headers[col], count_start+row);

			col_vals[col] = value;
		}
		
		list_row_add(list, col_vals);
		
		for (col = 0; col < _TEST_COLS; col++) {
			free(col_vals[col]);
		}

		free(col_vals);
	}

	gtk_statusbar_msg("Test data added.");
}

void ui_menu_debug_addtestrows_cb(void) {
	char *add_nr_of_rows_text = NULL;
	int add_nr_of_rows = 0;
	int col, row;
	char *col_headers[] = {
		"Col A", 
		"Col B", 
		"Col C", 
		"Col D", 
		"Col E"
	};
	char **col_vals;
	int count_start = list->nr_of_rows;
	
	add_nr_of_rows_text = gtk_input_dialog("Add how many rows?", "50");
	if (add_nr_of_rows_text == NULL) {
		return;
	}
	add_nr_of_rows = atoi(add_nr_of_rows_text);
	free(add_nr_of_rows_text);
	
	for (row = 0; row < add_nr_of_rows; row++) {
		col_vals = malloc(sizeof(void *) * _TEST_COLS);
		
		for (col = 0; col < _TEST_COLS; col++) {
			char *value = NULL;
			
			value = malloc(sizeof(char) * (strlen(col_headers[col] + 2)));
			sprintf(value, "%s-%02i", col_headers[col], count_start+row);

			col_vals[col] = value;
		}
		
		list_row_add(list, col_vals);
		
		for (col = 0; col < _TEST_COLS; col++) {
			free(col_vals[col]);
		}

		free(col_vals);
	}

	gtk_statusbar_msg("Added %i test rows.", add_nr_of_rows);
	
}

/* List *********************************************************************/
void ui_cell_edited_cb(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	gchar *old_text;
	GtkTreeIter iter;
	int col;
	
	/* Get column number */
	gtk_tree_view_get_cursor(treeview, &path, &column);
	col = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "col_nr"));
	
	/* Set new text */
	gtk_tree_model_get_iter(GTK_TREE_MODEL(list->liststore), &iter, path);
	gtk_tree_model_row_changed(GTK_TREE_MODEL(list->liststore), path, &iter);
	
	gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, col, &old_text, -1);
	if (old_text != NULL) {
		g_free(old_text);
	}
	
	gtk_list_store_set(GTK_LIST_STORE(list->liststore), &iter, col, new_text, -1);

	list->modified = TRUE;

	if (path != NULL) {
		gtk_tree_path_free(path);
	}
}


/****************************************************************************
 * User interface creation functions
 ****************************************************************************/
GtkWidget *ui_create_menubar(GtkWidget *window) {
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	
	accel_group = gtk_accel_group_new();
	
	item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);
	
	gtk_item_factory_create_items(item_factory, ui_nmenu_items, ui_menu_items, NULL);
	
	gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
	
	return gtk_item_factory_get_widget(item_factory, "<main>");
}

GtkWidget *ui_create_statusbar(GtkWidget *window) {

	sb_status = gtk_statusbar_new();
	sb_context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(sb_status), "main");
	gtk_statusbar_msg("Ready.");

	return (sb_status);
}
void dialog_about_btn_ok_cb(GtkWidget *widget, GtkWidget *win) {
	gtk_widget_destroy(win);
}


GtkWidget *ui_create_tree_view(void) {
	GtkTreeSelection *treeselection;

	treeview = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_signal_connect(
			GTK_OBJECT(treeview),
			"cursor-changed",
			G_CALLBACK(ui_treeview_cursor_changed_cb),
			NULL);

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), 1);
	treeselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
//	gtk_tree_selection_set_mode(treeselection, GTK_SELECTION_SINGLE);
	gtk_tree_selection_set_mode(treeselection, GTK_SELECTION_MULTIPLE);

	return (GTK_WIDGET(treeview));
}

void ui_menu_help_about_cb(void) {
	GtkWidget *win, *pixmapwid;
	GdkPixmap *logo;
	GtkStyle *style;
	GtkWidget *label;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *btn_ok;
	GdkBitmap *mask;
	
	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(win), "About ListPatron");
	gtk_widget_realize(win);

	style = gtk_widget_get_style( win );
    logo = gdk_pixmap_create_from_xpm_d(
			win->window,  
			&mask,
			&style->bg[GTK_STATE_NORMAL],
			(gchar **)splash_xpm );
    pixmapwid = gtk_pixmap_new( logo, mask );
    gtk_widget_show( pixmapwid );

	frame = gtk_frame_new(NULL);
	btn_ok = gtk_button_new_with_mnemonic("_Whatever");
	
	label = gtk_label_new("\nListPatron v%%VERSION\n\nCopyright, 2004, by Ferry Boender\n\n%%HOMEPAGE\nReleased under the GPL\n<%%EMAIL>");
	vbox = gtk_vbox_new(FALSE, 0);
	
	gtk_signal_connect(
			GTK_OBJECT(btn_ok),
			"clicked",
			GTK_SIGNAL_FUNC(dialog_about_btn_ok_cb),
			win);

	gtk_box_pack_start(GTK_BOX(vbox), pixmapwid, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), btn_ok, TRUE, TRUE, 5);
	
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_container_add(GTK_CONTAINER(win), frame);

	gtk_widget_show_all(win);

}

void ui_listtitle_click_cb(GtkWidget *widget, GdkEventButton *event, gpointer *data) {
	if (event->type == GDK_2BUTTON_PRESS) {
		char *new_title;
		new_title = gtk_input_dialog("Enter a name for the list", list->title);
		if (new_title != NULL) {
			list_title_set(new_title);
			free(new_title);
		}
    }
}

GtkWidget *ui_create_listtitle(void) {
	GtkWidget *eventbox;
	char *title;
	
	eventbox = gtk_event_box_new();
	lbl_listtitle = gtk_label_new(NULL);

	gtk_container_add(GTK_CONTAINER(eventbox), lbl_listtitle);
	
	g_signal_connect(
			eventbox, 
			"button-press-event", 
			(GCallback) ui_listtitle_click_cb, 
			NULL);
	
	title = strdup (list->title);
	list_title_set(title);
	free (title);
	
	return (eventbox);
}

void handle_cmdline_help() {
	if (opt_verbose) {
		printf("\
    _         _                     _    \n\
 __| |___ _ _| |_   _ __  __ _ _ _ (_)__ \n\
/ _` / _ \\ ' \\  _| | '_ \\/ _` | ' \\| / _|\n\
\\__,_\\___/_||_\\__| | .__/\\__,_|_||_|_\\__|\n\
                   |_|                   \n\
\n");
	}
	printf("\
ListPatron, version %%VERSION. (C) F.Boender, 2004. GPL\n\
Usage:  listpatron [option] file.lip\n\n\
Options:\n\
  -h, --help     Show this screen\n\
  -b, --batch    Batch-mode (No GUI)\n\
  -v, --version  Show versionumber\n\
      --verbose  Be verbose, show lots-o-output\n");

	exit(0);
}

void handle_cmdline(int argc, char *argv[]) {
	struct option long_options[] = {
		{"help"    , 0 , &opt_help    , 1}, 
		{"batch"   , 0 , &opt_batch   , 1}, 
		{"verbose" , 0 , &opt_verbose , 1}, 
		{"version" , 0 , &opt_version , 1}, 
		{0         , 0 , 0            , 0},
	};
	int c;

	while (1) {
		int option_index = 0;

		c = getopt_long(argc, argv, "hbv", long_options, &option_index);

		if (c == -1) {
			break;
		}
		
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0) {
					break;
				}
				break;
				
			case 'h': opt_help = 1; break;
			case 'b': opt_batch = 1; break;
			case 'v': opt_version = 1; break;
			case '?': opt_help = 1; break;

			default: abort(); break;
		}
	}

	if (opt_help) handle_cmdline_help();
	if (opt_version) printf("ListPatron, version %%VERSION. (C) F.Boender, 2004. GPL\n");

	/* Remaining option (open as listpatron file) */
	if (optind < argc) {
		list_load(list, argv[optind]);
	}
}

/****************************************************************************
 * Main
 ****************************************************************************/
int main(int argc, char *argv[]) {
	GtkWidget *vbox_main;
	GtkWidget *win_scroll;

	gtk_init(&argc, &argv);
	g_type_init();

	list = list_create();

	win_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(win_main), 500, 400);
	
	vbox_main = gtk_vbox_new(FALSE, 2);

	win_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(win_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(win_scroll), ui_create_tree_view());
	
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_menubar(win_main)), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_listtitle()), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(win_scroll), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_statusbar(win_main)), FALSE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(win_main), vbox_main);

	/* FIXME: Error dialogs occuring during handle_cmdline aren't shown */
	handle_cmdline(argc, argv);

	if (!opt_batch) {
		gtk_widget_show_all(win_main);
	
		gtk_main();
	}
	
	return (0);
}
