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

#include "listpatron.h"

/* Libs */
#include "debug.h"
#include "list.h"

/* Data */
#include "splash.h"
#include "menu_def.h"

/* User interface */
#include "ui_find.h"
#include "ui_import.h"
#include "ui_export.h"
#include "ui_rulelist.h"
#include "ui_sort.h"

GtkWidget *win_main;
GtkWidget *lbl_listtitle;
guint sb_context_id;
GtkWidget *sb_status;
GtkTreeView *treeview;
int merge_id_sorts;

/* Menu stuff (global because of dynamic menu's */
GtkActionGroup *action_group;
GtkUIManager *ui_manager;

extern list_ *list;

int 
	opt_help,
	opt_batch,
	opt_verbose,
	opt_version;

/* Tree and list */
void ui_treeview_cursor_changed_cb(GtkTreeView *tv, gpointer user_data) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	char *path_str;
	int col, row;

	/* Update the statusbar */
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

/****************************************************************************
 * Random wrappers that should have their own files but don't
 ****************************************************************************/
int ui_file_load(char *filename) {
	int err_nr;

	list_clear(list);
	list = list_create();

	if ((err_nr = list_load(list, filename)) != 0) {
		switch (err_nr) {
			case -1: gtk_error_dialog("Couldn't open file '%s'.", filename); break;
			case -2: gtk_error_dialog("Invalid listpatron file '%s'.", filename); break;
			case -3: gtk_error_dialog("Corrupt listpatron file '%s'.\n\nSome or all of the information in the file may have been lost.", filename); break;
			default: gtk_error_dialog("Unknown error while opening file '%s'.", filename); break;	
		}
	} else {
		gtk_statusbar_msg("File '%s' loaded.", filename);
		/* FIXME: Basename?? */
		list_filename_set(list, filename);
	}

	return(0);
}

/****************************************************************************
 * Callbacks (Menu)
 ****************************************************************************/
void ui_menu_file_new_cb(void) {
	list_clear(list);
	list = list_create();
	list_title_set(list, "Untitled");
	list->modified = FALSE; /* Overwrite mod flag setting by list_title_set */
}

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

		filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_open)));

		ui_file_load(filename);
		ui_create_menu_sortrules(list->sorts);

		free(filename);
	}

	gtk_widget_destroy(dia_file_open);
}

void ui_menu_file_import_csv_cb(void) {
	ui_import_csv();
}

void ui_menu_file_export_csv_cb(void) {
	ui_export_csv();
}

void ui_menu_file_export_ps_cb(void) {
	ui_export_ps();
}

void ui_menu_file_export_html_cb(void) {
	ui_export_html();
}

void ui_menu_file_save_cb(void) {
	GtkWidget *dia_file_save;
	int response;

	assert (list->title != NULL);

	if (list->filename == NULL) {
		/* No filename known; ask for one */
		char *cur_filename;
		char *new_filename;

		cur_filename = malloc(sizeof(char) * (strlen(list->title) + 5));
		sprintf (cur_filename, "%s.lip", list->title);
		
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
				cur_filename);

		free (cur_filename);
		
		response = gtk_dialog_run(GTK_DIALOG(dia_file_save));

		if (response == GTK_RESPONSE_ACCEPT) {
			new_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_save));
			list_save(list, new_filename);
			gtk_widget_destroy(dia_file_save);
		} else {
			gtk_widget_destroy(dia_file_save);
			return;
		}
	}
}

void ui_menu_file_save_as_cb(void) {
		GtkWidget *dia_file_save;
	int response;
	char *base_filename;
	char *new_filename;

	assert (list->title != NULL);

	if (list->filename != NULL) {
		/* Use the current filename as a basis for the new filename */
		base_filename = strdup(list->filename);
	} else {
		/* Use title as default filename */
		base_filename = malloc(sizeof(char) * (strlen(list->title) + 5));
		sprintf (base_filename, "%s.lip", list->title);
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
			base_filename);

	free(base_filename);

	response = gtk_dialog_run(GTK_DIALOG(dia_file_save));

	if (response == GTK_RESPONSE_ACCEPT) {
		new_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_save));
		list_save(list, new_filename);
		gtk_widget_destroy(dia_file_save);
	} else {
		gtk_widget_destroy(dia_file_save);
		return;
	}
}

void ui_menu_file_quit_cb(void) {
	int response;

	response = list_save_check(list);
	if (response != -1) {
		gtk_main_quit();
	}
}

void ui_menu_edit_find_cb(void) {
	ui_find();
}

void ui_menu_sort_rules_cb(void) {
	ui_rulelist(
			"Edit sorting rules", 
			"Sorting rule",
			list->sorts,
			ui_sort_new,
			ui_sort_edit,
			ui_sort_delete);
	ui_create_menu_sortrules(list->sorts);
}

void ui_menu_sort_edit_cb(void) {
	ui_sort_rule_edit(NULL);
}

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
	int col_nr;

	gtk_tree_view_get_cursor(treeview, &path, &column);
	if (path != NULL) {
		gtk_tree_path_free(path);
	}

	if (column == NULL) {
		return;
	}

	col_nr = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "col_nr"));


	column_name = gtk_input_dialog(
			"Enter the column name", 
			(char *)gtk_tree_view_column_get_title(column));

	if (column_name) {
		list_column_rename(col_nr, column_name);
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
	g_signal_connect(
			treeview,
			"cursor-changed",
			G_CALLBACK(ui_treeview_cursor_changed_cb),
			NULL);

	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), 1);
	treeselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
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
	
	g_signal_connect(
			btn_ok,
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
			list_title_set(list, new_title);
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
	list_title_set(list, title);
	list->modified = FALSE;
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
		ui_file_load(argv[optind]);
	}
}

void ui_menu_sort_rule_activate_cb(GtkAction *action, char *sort_rule) {
	sort_ *sort;
	GList *columns = NULL;
	GList *col_iter = NULL;

	/* Set the selected sort as the active sort */
	sort = list_sort_getrule(list, sort_rule);
	list->sort_active = sort->columns;
	
	/* Override default search func with our own */
	gtk_tree_sortable_set_default_sort_func(
			GTK_TREE_SORTABLE(list->liststore), 
			list_sort_func,
			list,
			NULL);
	gtk_tree_sortable_set_sort_column_id(
			GTK_TREE_SORTABLE(list->liststore),
			GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
			GTK_SORT_ASCENDING /* FIXME: Does it matter? */);

	/* Set column sort direction arrow indicators */
	columns = gtk_tree_view_get_columns(treeview);
	col_iter = columns;

	while (col_iter != NULL) {
		int col_nr;
		int col_sort_dir;
		sort_col_ *sort_col;
		int i;
		
		col_nr = GPOINTER_TO_UINT(
				g_object_get_data(G_OBJECT(col_iter->data), "col_nr")
		);
		
		/* Find the sort_order for this column */
		for (i = 0; i < sort->columns->len; i++) {
			sort_col = g_array_index(
					sort->columns,
					sort_col_ *,
					i);
			if (sort_col->col_nr == col_nr) {
				break;
			}
		}
		
		col_sort_dir = sort_col->sort_order;

		gtk_tree_view_column_set_sort_indicator(
				GTK_TREE_VIEW_COLUMN(col_iter->data),
				TRUE);
		gtk_tree_view_column_set_sort_order(
				GTK_TREE_VIEW_COLUMN(col_iter->data),
				col_sort_dir);
		
		
		col_iter = col_iter->next;
	}

}

void ui_create_menu_sortrules(GArray *rules) {
	int i;

	assert(rules != NULL);
	
	/* Remove old sorting rules */
	gtk_ui_manager_remove_ui(ui_manager, merge_id_sorts);

	merge_id_sorts = gtk_ui_manager_new_merge_id(ui_manager);

	/* Populate the sort rules list menu with rules */
	for (i = 0; i < rules->len; i++) {
		rule_ *rule;
		GtkAction *action;
		char *action_name = malloc(sizeof(char) * (8 + 20));

		rule = g_array_index(rules, rule_ *, i);
		
		sprintf(action_name, "SortRule%i", i);

		action = gtk_action_new(
				action_name,
				rule->name,
				"Sort by this rule",
				NULL);
		g_signal_connect(
				action,
				"activate",
				GTK_SIGNAL_FUNC(ui_menu_sort_rule_activate_cb),
				rule->name);

		gtk_action_group_add_action(
				action_group,
				action);

		gtk_ui_manager_add_ui(
				ui_manager,
				merge_id_sorts,
				"/MainMenu/DataMenu/SortMenu",
				action_name,
				action_name,
				GTK_UI_MANAGER_MENUITEM,
				FALSE);
	}
}

GtkWidget *ui_create_menu(void) {
	GtkWidget *menubar;
	GError *error;
	GtkAccelGroup *accel_group;

	action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), win_main);

	ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (win_main), accel_group);

	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error))
	  {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	  }

	menubar = gtk_ui_manager_get_widget(ui_manager, "/MainMenu");
	return (menubar);
}

GtkWidget *ui_create_toolbar(void) {
	GtkWidget *toolbar;
	GError *error;
	GtkAccelGroup *accel_group;

	action_group = gtk_action_group_new ("ToolbarActions");
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), win_main);

	ui_manager = gtk_ui_manager_new ();
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);

	accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (win_main), accel_group);

	error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description_toolbar, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}

	toolbar = gtk_ui_manager_get_widget(ui_manager, "/ToolbarFile");

	return (toolbar);
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
	g_signal_connect (GTK_OBJECT(win_main), "delete-event", G_CALLBACK(ui_menu_file_quit_cb), NULL);
	
	vbox_main = gtk_vbox_new(FALSE, 0);

	win_scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(win_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(win_scroll), ui_create_tree_view());
	
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_menu()),  FALSE,TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_toolbar()), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_listtitle()), FALSE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(win_scroll), TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(vbox_main), GTK_WIDGET(ui_create_statusbar(win_main)), FALSE, TRUE, 0);

	gtk_container_add(GTK_CONTAINER(win_main), vbox_main);

	/* FIXME: Error dialogs occuring during handle_cmdline aren't shown */
	handle_cmdline(argc, argv);

	ui_create_menu_sortrules(list->sorts);

	if (!opt_batch) {
		gtk_widget_show_all(win_main);
		gtk_main();
	}

	return (0);
}

