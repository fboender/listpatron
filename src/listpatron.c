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
 * Copyright (C) 2002 Ferry Boender.
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
#include <gtk/gtk.h>
#include <glib-object.h>
#include <libxml/parser.h>

#define _DEBUG
#define _TEST_COLS 5
#define _TEST_ROWS 50


/****************************************************************************/

typedef struct list_ {
	char *filename;
	GtkTreeView *treeview;
	GtkListStore *liststore;
	int nr_of_cols;
	int nr_of_rows;
} list_;

/****************************************************************************
 * Prototyping
 ****************************************************************************/
/* List handling functions */
void list_column_add (list_ *list, char *title);
void list_column_delete (list_ *list, GtkTreeViewColumn *column);
void list_row_add_empty (list_ *list);
void list_row_add (list_ *list, int nr_of_cols, char *values[]);
list_ *list_create (void);
int list_load (list_ *list, char *filename);
int list_save (list_ *list, char *filename);
/* Callback functions */
void ui_menu_file_open_cb (void);
void ui_menu_file_save_cb (void);
void ui_menu_file_save_as_cb (void);
void ui_menu_column_add_cb (void);
void ui_menu_column_rename_cb (void);
void ui_menu_column_delete_cb (void);
void ui_menu_row_add_cb (void);
void ui_menu_row_delete_cb (void);
void ui_menu_debug_addtestdata_cb (void);
void ui_menu_debug_addtestrows_cb (void);
void ui_cell_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data);
/* User interface creation functions */
GtkWidget *ui_create_menubar (GtkWidget *window);

/****************************************************************************
 * Data initializer
 ****************************************************************************/
static GtkItemFactoryEntry ui_menu_items[] = {
	{ "/_File"               , NULL, NULL                        , 0, "<Branch>"                      },
	{ "/File/_New"           , NULL, list_column_add             , 0, "<StockItem>" , GTK_STOCK_NEW   },
	{ "/File/_Open"          , NULL, ui_menu_file_open_cb        , 0, "<StockItem>" , GTK_STOCK_OPEN  },
	{ "/File/_Save"          , NULL, ui_menu_file_save_cb        , 0, "<StockItem>" , GTK_STOCK_SAVE  },
	{ "/File/Save _As"       , NULL, ui_menu_file_save_as_cb     , 0, "<Item>"                        },
	{ "/File/_Export"        , NULL, NULL                        , 0, "<Item>"                        },
	{ "/File/sep1"           , NULL, NULL                        , 0, "<Separator>"                   },
	{ "/File/_Quit"          , NULL, gtk_main_quit               , 0, "<StockItem>" , GTK_STOCK_QUIT  },
	{ "/_Edit"               , NULL, NULL                        , 0, "<Branch>"                      },
	{ "/Edit/Cu_t"           , NULL, NULL                        , 0, "<StockItem>" , GTK_STOCK_CUT   },
	{ "/Edit/_Copy"          , NULL, NULL                        , 0, "<StockItem>" , GTK_STOCK_COPY  },
	{ "/Edit/_Paste"         , NULL, NULL                        , 0, "<StockItem>" , GTK_STOCK_PASTE },
	{ "/Edit/sep1"           , NULL, NULL                        , 0, "<Separator>"                   },
	{ "/_Column"             , NULL, NULL                        , 0, "<Branch>"                      },
	{ "/Column/_Add"         , NULL, ui_menu_column_add_cb       , 0, "<StockItem>" , GTK_STOCK_ADD   },
	{ "/Column/_Delete"      , NULL, ui_menu_column_delete_cb    , 0, "<StockItem>" , GTK_STOCK_DELETE},
	{ "/Column/_Rename"      , NULL, ui_menu_column_rename_cb    , 0, "<Item>"                        },
	{ "/_Row"                , NULL, NULL                        , 0, "<Branch>"                      },
	{ "/Row/_Add"            , NULL, ui_menu_row_add_cb          , 0, "<StockItem>" , GTK_STOCK_ADD   },
	{ "/Row/_Delete"         , NULL, ui_menu_row_delete_cb       , 0, "<StockItem>" , GTK_STOCK_DELETE},
#ifdef _DEBUG
	{ "/_Debug"              , NULL, NULL                        , 0, "<Branch>"                      },
	{ "/Debug/_Add test data", NULL, ui_menu_debug_addtestdata_cb, 0, "<Item>"                        },
	{ "/Debug/Add test _rows", NULL, ui_menu_debug_addtestrows_cb, 0, "<Item>"                        },
#endif
	{ "/_Help"               , NULL, NULL                        , 0, "<LastBranch>"                  },
	{ "/_Help/About"         , NULL, NULL                        , 0, "<Item>"                        },
};

static gint ui_nmenu_items = sizeof (ui_menu_items) / sizeof (ui_menu_items[0]);

list_ *list = NULL;

/****************************************************************************
 * gtk_input_dialog
 ****************************************************************************/
char *gtk_input_dialog (char *message, char *prefill) {
	GtkWidget *dia_input;
	GtkWidget *ent_input;
	gint result;
	char *response = NULL;
	
	dia_input = gtk_dialog_new_with_buttons(
			message,
			NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			NULL);
	
	ent_input = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY(ent_input), prefill);

	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dia_input)->vbox), GTK_WIDGET(ent_input), FALSE, TRUE, 0);
	
	gtk_widget_show_all (dia_input);

	result = gtk_dialog_run (GTK_DIALOG(dia_input));
	switch (result) {
		case GTK_RESPONSE_ACCEPT: 
			response = strdup((char *)gtk_entry_get_text (GTK_ENTRY(ent_input)));
			break;
		default:
			break;
	}
	gtk_widget_destroy (dia_input);

	return (response);
}

/****************************************************************************
 * List handling functions
 ****************************************************************************/
void list_column_add (list_ *list, char *title) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GType *types = NULL;
	GtkListStore *liststore_old = NULL;
	int i;
	
	/* TreeView */
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	col = gtk_tree_view_column_new_with_attributes(
			title, 
			renderer,
			"text", list->nr_of_cols,
			NULL);
	g_object_set_data (G_OBJECT(col), "col_nr", GUINT_TO_POINTER(list->nr_of_cols));

	g_signal_connect (renderer, "edited", (GCallback) ui_cell_edited_cb, NULL);

	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_sort_column_id (GTK_TREE_VIEW_COLUMN(col), list->nr_of_cols);

	gtk_tree_view_append_column (list->treeview, col);

	/* ListStore: Rebuild from scratch because columns can't be added */

	/* Remember the old liststore */
	if (list->liststore != NULL) {
		liststore_old = list->liststore;
	}
	
	/* Create new liststore from scratch */
	types = malloc(sizeof(GType) * (list->nr_of_cols + 1));
	for (i = 0; i < (list->nr_of_cols + 1); i++) {
		types[i] = G_TYPE_STRING;
	}
	list->liststore = gtk_list_store_newv (list->nr_of_cols + 1, types);
	free (types);

	/* Copy data from previous liststore (if existed) and clean up old store */
	if (liststore_old != NULL) {
		gchar *row_data;
		GtkTreeIter iter_old;
		GtkTreeIter iter;
		
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL(liststore_old), &iter_old)) {
			do {
				gtk_list_store_append (list->liststore, &iter);
				
				/* Copy old liststore to new liststore */
				for (i = 0; i < list->nr_of_cols; i++) {
					gtk_tree_model_get (GTK_TREE_MODEL(liststore_old), &iter_old, i, &row_data, -1);
					gtk_list_store_set (list->liststore, &iter, i, row_data, -1);
					
				}
				gtk_list_store_set (list->liststore, &iter, list->nr_of_cols, "", -1);
			} while (gtk_tree_model_iter_next (GTK_TREE_MODEL(liststore_old), &iter_old));

			/* Clean up old liststore */
			gtk_list_store_clear (liststore_old);
			
		}
	}

	gtk_tree_view_set_model (list->treeview, GTK_TREE_MODEL(list->liststore));
	
	list->nr_of_cols++;
}

void list_column_delete (list_ *list, GtkTreeViewColumn *column) {
	int liststore_remove_col_nr = -1;
	GList *columns = NULL;
	GType *types = NULL;
	GtkListStore *liststore_old = NULL;
	int i;

	if (column == NULL) {
		return;
	}
	
	liststore_remove_col_nr = GPOINTER_TO_UINT(g_object_get_data (G_OBJECT(column), "col_nr"));

	/* Treeview: Remove column and update other columns to compensate for 
	 * shifting in liststore */
	columns = gtk_tree_view_get_columns (list->treeview);
	while (columns != NULL) {
		int col_nr;

		col_nr = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(columns->data), "col_nr"));

		if (col_nr > liststore_remove_col_nr) { /* Shift columns to left */
			GList *col_renderers = NULL;

			col_renderers = gtk_tree_view_column_get_cell_renderers (columns->data);
			gtk_tree_view_column_set_attributes (columns->data, col_renderers->data, "text", col_nr - 1, NULL);
			gtk_tree_view_column_set_sort_column_id (columns->data, col_nr - 1);
				
			g_object_set_data(G_OBJECT(columns->data), "col_nr", GUINT_TO_POINTER(col_nr -1));
			
		}
		columns = columns->next;
	}
	
	gtk_tree_view_remove_column (list->treeview, column);
		
	/* Rewrite liststore from scratch because GTK won't allow us to remove 
	 * columns */
	/* Remember old liststore so we can copy data from it */
	if (list->liststore != NULL) {
		liststore_old = list->liststore;
	}

	if (list->nr_of_cols - 1 > 0) {
		/* Create new liststore from scratch */
		types = malloc(sizeof(GType) * (list->nr_of_cols - 1));
		for (i = 0; i < (list->nr_of_cols - 1); i++) {
			types[i] = G_TYPE_STRING;
		}
		list->liststore = gtk_list_store_newv (list->nr_of_cols - 1, types);
		free (types);
		
		if (liststore_old != NULL) {
			gchar *row_data;
			GtkTreeIter iter_old;
			GtkTreeIter iter;
			
			if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL(liststore_old), &iter_old)) {
				do {
					int col_counter = 0;

					gtk_list_store_append (list->liststore, &iter);
					
					/* Copy old liststore to new liststore */
					for (i = 0; i < list->nr_of_cols; i++) {
						if (i != liststore_remove_col_nr) {
							gtk_tree_model_get (GTK_TREE_MODEL(liststore_old), &iter_old, i, &row_data, -1);
							gtk_list_store_set (list->liststore, &iter, col_counter, row_data, -1);
							col_counter++;
						}
					}
				} while (gtk_tree_model_iter_next (GTK_TREE_MODEL(liststore_old), &iter_old));

				/* Clean up old liststore */
				gtk_list_store_clear (liststore_old);
			}
		}
		
		gtk_tree_view_set_model (list->treeview, GTK_TREE_MODEL(list->liststore));
	}

	list->nr_of_cols--;
}

void list_row_add_empty (list_ *list) {
	GtkTreeIter treeiter;
	int i;

	if (list->liststore == NULL) {
		return;
	}
	
	gtk_list_store_append (list->liststore, &treeiter);
	for (i = 0; i < list->nr_of_cols; i++) {
		gtk_list_store_set (list->liststore, &treeiter, i, "", -1);
	}

	list->nr_of_rows++;
}

void list_row_add (list_ *list, int nr_of_cols, char *values[]) {
	GtkTreeIter treeiter;
	int i;

	gtk_list_store_append (list->liststore, &treeiter);
	for (i = 0; i < nr_of_cols; i++) {
		gtk_list_store_set (list->liststore, &treeiter, i, values[i], -1);
	}

	list->nr_of_rows++;
}

list_ *list_create (void) {
	list_ *list = NULL;
	GtkTreeSelection *treeselection;
	list = malloc(sizeof(list_));

	list->filename   = NULL;
	list->treeview   = NULL;
	list->liststore  = NULL;
	list->nr_of_cols = 0;
	list->nr_of_rows = 0;

	list->treeview  = GTK_TREE_VIEW (gtk_tree_view_new());
	
	treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(list->treeview));
	gtk_tree_selection_set_mode (treeselection, GTK_SELECTION_SINGLE);
	
	return (list);
}

int list_load (list_ *list, char *filename) {
	xmlDocPtr doc;
	xmlNodePtr node_header, node_rowdata;
	xmlNodePtr node_cols, node_rows;
	char **rowdata;
	
	doc = xmlReadFile (filename, "ISO-8859-1", 0);

	node_header = doc->children->children->next;
	node_rowdata = doc->children->children->next->next->next;

	/* Read columns and adjust list */
	list->nr_of_cols = 0;
	node_cols = node_header->children;

	while (node_cols != NULL) {
		if (node_cols->type == XML_ELEMENT_NODE) {
			list_column_add (list, node_cols->children->content);
		}
		node_cols = node_cols->next;
	}

	/* Read rows and add to list */
	rowdata = malloc (sizeof(char *) * list->nr_of_cols);
	node_rows = node_rowdata->children;
	
	while (node_rows != NULL) {
		if (node_rows->type == XML_ELEMENT_NODE) {
			int i = 0;
			node_cols = node_rows->children;
			while (node_cols != NULL) {
				if (node_cols->type == XML_ELEMENT_NODE) {
					rowdata[i] = node_cols->children->content;
					i++;
				}
				node_cols = node_cols->next;
			}

			list_row_add (list, list->nr_of_cols, rowdata);
		}
		node_rows = node_rows->next;
	}

	free (rowdata);

	list->filename = strdup(filename);

	return (0);
}

int list_save (list_ *list, char *filename) {
	xmlDocPtr doc;
	xmlNodePtr node_root, node_header, node_rowdata, node_row;
	GtkTreeIter iter;
	gchar *row_data;
	GList *columns = NULL, *column_iter = NULL;
	int i;
	
	/* Create XML document */
	doc = xmlNewDoc ("1.0");
	node_root = xmlNewDocNode(doc, NULL, (const xmlChar *)"list", NULL);
	xmlDocSetRootElement (doc, node_root);

	/* Parse column information */
	node_header = xmlNewChild (node_root, NULL, (const xmlChar *)"header", NULL);
	
	columns = gtk_tree_view_get_columns (list->treeview);
	column_iter = columns;
	while (column_iter) {
		xmlNewChild(node_header, NULL, (const xmlChar *)"col", (char *)GTK_TREE_VIEW_COLUMN(column_iter->data)->title);
		column_iter = column_iter->next;
	}

	/* Parse row information */
	node_rowdata = xmlNewChild (node_root, NULL, (const xmlChar *)"rowdata", NULL);
	
	gtk_tree_model_get_iter_root (GTK_TREE_MODEL(list->liststore), &iter);
	do {
		node_row = xmlNewChild (node_rowdata, NULL, (const xmlChar *)"row", NULL);
		for (i = 0; i < list->nr_of_cols; i++) {
			gtk_tree_model_get (GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			xmlNewChild(node_row, NULL, (const xmlChar *)"col", row_data);
			free (row_data);
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));
	
	xmlSaveFormatFileEnc (filename, doc, "ISO-8859-1", 1);
	
	list->filename = strdup(filename);
	return (0);
}

/****************************************************************************
 * Callbacks 
 ****************************************************************************/
/* File open callbacks */

/* Menu *********************************************************************/
void ui_file_open_btn_ok_cb (GtkWidget *win, GtkFileSelection *fs) {
	char *filename = NULL;
	
	filename = (char *)gtk_file_selection_get_filename (GTK_FILE_SELECTION(fs));
	
	list_load (list, filename);
}

void ui_menu_file_open_cb (void) {
	GtkWidget *win_file_open;

	win_file_open = gtk_file_selection_new ("Open file");
	
    g_signal_connect (
			G_OBJECT (GTK_FILE_SELECTION (win_file_open)->ok_button),
			"clicked", 
			G_CALLBACK (ui_file_open_btn_ok_cb), 
			(gpointer) win_file_open);
    g_signal_connect_swapped (
			G_OBJECT (GTK_FILE_SELECTION (win_file_open)->ok_button),
			"clicked", 
			G_CALLBACK (gtk_widget_destroy), 
			G_OBJECT (win_file_open));
    g_signal_connect_swapped (
			G_OBJECT (GTK_FILE_SELECTION (win_file_open)->cancel_button),
			"clicked", 
			G_CALLBACK (gtk_widget_destroy), 
			G_OBJECT (win_file_open));
	
	gtk_widget_show (win_file_open);
}

void ui_file_save_btn_ok_cb (GtkWidget *win, GtkFileSelection *fs) {
	char *filename = NULL;
	
	filename = (char *)gtk_file_selection_get_filename (GTK_FILE_SELECTION(fs));
	
	list_save (list, filename);
}


void ui_menu_file_save_cb (void) {
	GtkWidget *win_file_save;

	if (list->filename == NULL) {
		win_file_save = gtk_file_selection_new ("Save file");
		
		g_signal_connect (
				G_OBJECT (GTK_FILE_SELECTION (win_file_save)->ok_button),
				"clicked", 
				G_CALLBACK (ui_file_save_btn_ok_cb), 
				(gpointer) win_file_save);
		g_signal_connect_swapped (
				G_OBJECT (GTK_FILE_SELECTION (win_file_save)->ok_button),
				"clicked", 
				G_CALLBACK (gtk_widget_destroy), 
				G_OBJECT (win_file_save));
		g_signal_connect_swapped (
				G_OBJECT (GTK_FILE_SELECTION (win_file_save)->cancel_button),
				"clicked", 
				G_CALLBACK (gtk_widget_destroy), 
				G_OBJECT (win_file_save));
		
		gtk_widget_show (win_file_save);
	} else {
		list_save (list, list->filename);
	}
}

void ui_menu_file_save_as_cb (void) {
	char *old_filename;

	old_filename = list->filename;
	list->filename = NULL;
	ui_menu_file_save_cb();
	list->filename = old_filename;
}

void ui_menu_column_add_cb (void) {
	char *column_name = NULL;
	
	column_name = gtk_input_dialog ("Enter the column name", "Col");
	if (column_name) {
		list_column_add (list, column_name);
		free (column_name);
	}
}

void ui_menu_column_rename_cb (void) {
	char *column_name = NULL;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	
	column_name = gtk_input_dialog ("Enter the column name", "Col");
	if (column_name) {
		gtk_tree_view_get_cursor (list->treeview, &path, &column);
		gtk_tree_view_column_set_title (column, column_name);
		free (column_name);
	}
}

void ui_menu_column_delete_cb (void) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	
	gtk_tree_view_get_cursor (list->treeview, &path, &column);

	list_column_delete (list, column);
}

void ui_menu_row_add_cb (void) {
	list_row_add_empty (list);
}


void ui_menu_row_delete_cb (void) {
	GtkTreePath *path;
	GtkTreeViewColumn *column; /* Unused */
	GtkTreeIter iter;

	gtk_tree_view_get_cursor (list->treeview, &path, &column);
	
	if (path != NULL) { /* Row selected? */
		if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(list->liststore), &iter, path)) {
			return;
		}
		
		gtk_list_store_remove(GTK_LIST_STORE(list->liststore), &iter);
	}
	
}

void ui_menu_debug_addtestdata_cb (void) {
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
		list_column_add (list, col_headers[col]);
	}
	
	for (row = 0; row < _TEST_ROWS; row++) {
		col_vals = malloc(sizeof(void *) * _TEST_COLS);
		
		for (col = 0; col < _TEST_COLS; col++) {
			char *value = NULL;
			
			value = malloc(sizeof(char) * (strlen(col_headers[col] + 2)));
			sprintf (value, "%s-%02i", col_headers[col], count_start+row);

			col_vals[col] = value;
		}
		
		list_row_add (list, _TEST_COLS, col_vals);
		
		for (col = 0; col < _TEST_COLS; col++) {
			free (col_vals[col]);
		}

		free (col_vals);
	}
}

void ui_menu_debug_addtestrows_cb (void) {
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
	
	add_nr_of_rows_text = gtk_input_dialog ("Add how many rows?", "50");
	add_nr_of_rows = atoi (add_nr_of_rows_text);
	
	for (row = 0; row < add_nr_of_rows; row++) {
		col_vals = malloc(sizeof(void *) * _TEST_COLS);
		
		for (col = 0; col < _TEST_COLS; col++) {
			char *value = NULL;
			
			value = malloc(sizeof(char) * (strlen(col_headers[col] + 2)));
			sprintf (value, "%s-%02i", col_headers[col], count_start+row);

			col_vals[col] = value;
		}
		
		list_row_add (list, _TEST_COLS, col_vals);
		
		for (col = 0; col < _TEST_COLS; col++) {
			free (col_vals[col]);
		}

		free (col_vals);
	}

}

/* List *********************************************************************/
void ui_cell_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	gchar *old_text;
	GtkTreeIter iter;
	int col;
	
	/* Get column number */
	gtk_tree_view_get_cursor (list->treeview, &path, &column);
	col = GPOINTER_TO_UINT(g_object_get_data (G_OBJECT(column), "col_nr"));
	
	/* Set new text */
	gtk_tree_model_get_iter (GTK_TREE_MODEL(list->liststore), &iter, path);
	gtk_tree_model_row_changed (GTK_TREE_MODEL(list->liststore), path, &iter);
	
	gtk_tree_model_get (GTK_TREE_MODEL(list->liststore), &iter, col, &old_text, -1);
	if (old_text != NULL) {
		g_free (old_text);
	}
	
	gtk_list_store_set (GTK_LIST_STORE(list->liststore), &iter, col, new_text, -1);
}


/****************************************************************************
 * User interface creation functions
 ****************************************************************************/
GtkWidget *ui_create_menubar (GtkWidget *window) {
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	
	accel_group = gtk_accel_group_new ();
	
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", accel_group);
	
	gtk_item_factory_create_items (item_factory, ui_nmenu_items, ui_menu_items, NULL);
	
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	
	return gtk_item_factory_get_widget (item_factory, "<main>");
}

/****************************************************************************
 * Main
 ****************************************************************************/
int main (int argc, char *argv[]) {
	GtkWidget *win_main;
	GtkWidget *vbox_main;
	GtkWidget *win_scroll;

	gtk_init (&argc, &argv);
	g_type_init();

	win_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW(win_main), 400, 200);
	
	vbox_main = gtk_vbox_new (FALSE, 2);

	list = list_create();

	win_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(win_scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER(win_scroll), GTK_WIDGET(list->treeview));
	
	gtk_box_pack_start (GTK_BOX(vbox_main), GTK_WIDGET(ui_create_menubar(win_main)), FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox_main), GTK_WIDGET(win_scroll), TRUE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER(win_main), vbox_main);

	gtk_widget_show_all (win_main);
	gtk_main();
	
	return (0);
}
