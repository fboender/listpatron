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
#include <getopt.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "splash.h"

#define _DEBUG
#define _TEST_COLS 5
#define _TEST_ROWS 50

#define ORIENT_PORTRAIT 0
#define ORIENT_LANDSCAPE 1

/****************************************************************************/

typedef struct list_ {
	char *filename;

	/* Info */
	char *version;
	char *title;
	char *author;
	char *description;
	char *keywords;

	GtkTreeView *treeview;
	GtkListStore *liststore;
	int nr_of_cols;
	int nr_of_rows;
	int modified;
} list_;

typedef struct import_ {
	char *filename;
	char delimiter;
} import_;

typedef struct export_ {
	char *filename;
	int orientation;
} export_;


/* Character Separated Value's Import options */
#define IMPORT_CSV_HEADER         1 << 0 /* First row contains column header titles */
#define IMPORT_CSV_QUOTE_ENCLODED 1 << 1 /* Fields are enclosed in quotes (single or double) */

#define DBG_ERR 1
#define DBG_WARN 2

/****************************************************************************
 * Prototyping
 ****************************************************************************/
/* Personal GTK additions, sort of */
char *gtk_input_dialog(char *message, char *prefill);
void gtk_error_dialog(char *fmt, ...);

/* libxml extending functions */
xmlNodeSet *xml_get_nodeset(xmlDocPtr doc, char *xpath);
char *xml_get_element_content(xmlDocPtr doc, char *xpath);

/* List handling functions */
void list_column_add(list_ *list, char *title);
void list_column_delete(list_ *list, GtkTreeViewColumn *column);
void list_row_add_empty(list_ *list);
void list_row_add(list_ *list, int nr_of_cols, char *values[]);
list_ *list_create(void);
int list_import_csv(list_ *list, char *filename, char delimiter);
void ui_file_export_ps_portrait_cb(GtkWidget *radio, export_ *export);
void ui_file_export_ps_landscape_cb(GtkWidget *radio, export_ *export);
int list_export_ps(list_ *list, char *filename, int orientation);
int list_export_html(list_ *list, char *filename);
int list_load(list_ *list, char *filename);
int list_save(list_ *list, char *filename);

/* Menu callback functions */
void ui_menu_file_import_csv_cb(void);
void ui_menu_file_export_ps_cb(void);
void ui_menu_file_export_html_cb(void);
void ui_menu_file_open_cb(void);
void ui_menu_file_save_cb(void);
void ui_menu_file_save_as_cb(void);
void ui_menu_file_quit_cb(void);
void ui_menu_column_add_cb(void);
void ui_menu_column_rename_cb(void);
void ui_menu_column_delete_cb(void);
void ui_menu_row_add_cb(void);
void ui_menu_row_delete_cb(void);
void ui_menu_debug_addtestdata_cb(void);
void ui_menu_debug_addtestrows_cb(void);
void ui_menu_help_about_cb(void);

/* Various Callback functions */
void ui_treeview_cursor_changed_cb(GtkTreeView *tv, gpointer user_data);
void ui_cell_edited_cb(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data);

/* User interface creation functions */
GtkWidget *ui_create_menubar(GtkWidget *window);

/****************************************************************************
 * Data initializer
 ****************************************************************************/
static GtkItemFactoryEntry ui_menu_items[] = {
	{ "/_File"                            , NULL , NULL                         , 0 , "<Branch>"                      },
	{ "/File/_New"                        , NULL , list_column_add              , 0 , "<StockItem>", GTK_STOCK_NEW    },
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

static int 
	opt_help,
	opt_batch,
	opt_verbose,
	opt_version;

list_ *list = NULL;

/****************************************************************************
 * Debugging functions
 ****************************************************************************/
void debug_msg(int dbg_type, char *file, int line, char *fmt, ...) {
#ifdef _DEBUG
	va_list argp;
	char *err_usr = NULL, *err;
	int n = 10, err_len = 10;
	
	if (!opt_verbose) { 
		return; 
	}

	err_usr = malloc(err_len);

	while (n == err_len) { /* Keep trying until msg fits in the buffer */
		va_start(argp, fmt);
		n = vsnprintf(err_usr, err_len, fmt, argp);
		va_end(argp);
		
		if (n < -1) {
			return;
		} else 
		if (n > err_len) { /* Throw some more mem at the buf */
			err_len = (2 * err_len);
			n = err_len;
			err_usr = realloc(err_usr, err_len+1);
		} else {
			n = 0; /* That'll be enough, thank you */
		}
	}
	
	err = malloc(sizeof(char) * (12 + strlen(file) + 8 + strlen(err_usr) + 2));
	sprintf(err, "DEBUG: %s:%i : %s\n", file, line, err_usr);
	fprintf(stderr, err);
	free(err);

	free(err_usr);
#endif
}

/****************************************************************************
 * gtk_input_dialog
 ****************************************************************************/
void gtk_input_dialog_entry_activate_cb(GtkWidget *entry, GtkDialog *dia_input) {
	gtk_dialog_response(GTK_DIALOG(dia_input), GTK_RESPONSE_ACCEPT);
}

/* Free returned value */
char *gtk_input_dialog(char *message, char *prefill) {
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
	
	ent_input = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(ent_input), prefill);
	g_signal_connect(
		ent_input,
		"activate",
		(GCallback) gtk_input_dialog_entry_activate_cb,
		dia_input);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_input)->vbox), GTK_WIDGET(ent_input), FALSE, TRUE, 0);
	
	gtk_widget_show_all(dia_input);

	result = gtk_dialog_run(GTK_DIALOG(dia_input));
	switch (result) {
		case GTK_RESPONSE_ACCEPT: 
			response = strdup((char *)gtk_entry_get_text(GTK_ENTRY(ent_input)));
			break;
		default:
			response = NULL;
			break;
	}
	gtk_widget_destroy(dia_input);

	return (response);
}

void gtk_error_dialog(char *fmt, ...) {
	va_list argp;
	char *err = NULL;
	int n = 10, err_len = 10;
	GtkWidget *dia_error;
	
	err = malloc(err_len);

	/* I'd like to unify this into a single function, but it seems that can't 
	 * be done. I'm getting a '`va_start' used in function with fixed args'
	 * error. If anyone knows, please mail me */
	while (n == err_len) { /* Keep trying until msg fits in the buffer */
		va_start(argp, fmt);
		n = vsnprintf(err, err_len, fmt, argp);
		va_end(argp);
		
		if (n < -1) {
			return;
		} else 
		if (n > err_len) { /* Throw some more mem at the buf */
			err_len = (2 * err_len);
			n = err_len;
			err = realloc(err, err_len+1);
		} else {
			n = 0; /* That'll be enough, thank you */
		}
	}
	
	dia_error = gtk_message_dialog_new(
			GTK_WINDOW(win_main), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_ERROR, 
			GTK_BUTTONS_CLOSE, 
			err);

	free(err);
	
	gtk_dialog_run(GTK_DIALOG(dia_error));
	gtk_widget_destroy(dia_error);
}

void gtk_statusbar_msg(char *fmt, ...) {
	va_list argp;
	char *msg = NULL;
	int n = 10, msg_len = 10;
	
	msg = malloc(msg_len);

	/* I'd like to unify this into a single function, but it seems that can't 
	 * be done. I'm getting a '`va_start' used in function with fixed args'
	 * msgor. If anyone knows, please mail me */
	while (n == msg_len) { /* Keep trying until msg fits in the buffer */
		va_start(argp, fmt);
		n = vsnprintf(msg, msg_len, fmt, argp);
		va_end(argp);
		
		if (n < -1) {
			return;
		} else 
		if (n > msg_len) { /* Throw some more mem at the buf */
			msg_len = (2 * msg_len);
			n = msg_len;
			msg = realloc(msg, msg_len+1);
		} else {
			n = 0; /* That'll be enough, thank you */
		}
	}
	
	gtk_statusbar_push(GTK_STATUSBAR(sb_status), sb_context_id, msg);

	free(msg);
}

/****************************************************************************
 * libxml2 extending functions
 ****************************************************************************/
xmlNodeSet *xml_get_nodeset(xmlDocPtr doc, char *xpath){
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	context = xmlXPathNewContext(doc);
	result = xmlXPathEvalExpression(xpath, context);
	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
		return NULL;
	}
	xmlXPathFreeContext(context);

	return (result->nodesetval);
}

/* Return value exists only during existance of xml document */
char *xml_get_element_content(xmlDocPtr doc, char *xpath) {
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	
	context = xmlXPathNewContext(doc);
	result = xmlXPathEvalExpression(xpath, context);
	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
		return ("");
	}
	xmlXPathFreeContext(context);

	if (result->nodesetval->nodeTab[0]->children) {
		return (result->nodesetval->nodeTab[0]->children->content);
	} else {
		return ("");
	}
}

xmlNodePtr xml_add_element_content(xmlNodePtr node_parent, char *element_name, char *fmt, ...) {
	va_list argp;
	char *content = NULL;
	xmlNodePtr node_new = NULL;
	int n = 10, content_len = 10;
	
	content = malloc(content_len);

	/* I'd like to unify this into a single function, but it seems that can't 
	 * be done. I'm getting a '`va_start' used in function with fixed args'
	 * msgor. If anyone knows, please mail me */
	while (n == content_len) { /* Keep trying until contentfits in the buffer */
		va_start(argp, fmt);
		n = vsnprintf(content, content_len, fmt, argp);
		va_end(argp);
		
		if (n < -1) {
			return (NULL);
		} else 
		if (n > content_len) { /* Throw some more mem at the buf */
			content_len = (2 * content_len);
			n = content_len;
			content= realloc(content, content_len+1);
		} else {
			n = 0; /* That'll be enough, thank you */
		}
	}
	
	node_new = xmlNewChild(node_parent, NULL, element_name, (const xmlChar *)content);
	
	free(content);

	return (node_new);
}

/* NOTICE: Unused */
char *xml_element_get_value(xmlNodePtr node_parent, char *element_name) {
	if (node_parent->type == XML_ELEMENT_NODE) {
		if (strcmp(node_parent->name, element_name) == 0) {
			return (node_parent->children->content);
		}
	}

	return (NULL);
}

/****************************************************************************
 * List handling functions
 ****************************************************************************/
void list_column_add(list_ *list, char *title) {
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
	g_object_set_data(G_OBJECT(col), "col_nr", GUINT_TO_POINTER(list->nr_of_cols));

	g_signal_connect(renderer, "edited", (GCallback) ui_cell_edited_cb, NULL);

	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_reorderable(GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_sort_column_id(GTK_TREE_VIEW_COLUMN(col), list->nr_of_cols);

	gtk_tree_view_append_column(list->treeview, col);

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
	list->liststore = gtk_list_store_newv(list->nr_of_cols + 1, types);
	free(types);

	/* Copy data from previous liststore (if existed) and clean up old store */
	if (liststore_old != NULL) {
		gchar *row_data;
		GtkTreeIter iter_old;
		GtkTreeIter iter;
		
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore_old), &iter_old)) {
			do {
				gtk_list_store_append(list->liststore, &iter);
				
				/* Copy old liststore to new liststore */
				for (i = 0; i < list->nr_of_cols; i++) {
					gtk_tree_model_get(GTK_TREE_MODEL(liststore_old), &iter_old, i, &row_data, -1);
					gtk_list_store_set(list->liststore, &iter, i, row_data, -1);
					
				}
				gtk_list_store_set(list->liststore, &iter, list->nr_of_cols, "", -1);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore_old), &iter_old));

			/* Clean up old liststore */
			gtk_list_store_clear(liststore_old);
			
		}
	}

	gtk_tree_view_set_model(list->treeview, GTK_TREE_MODEL(list->liststore));
	
	list->nr_of_cols++;
	list->modified = TRUE;
	
}

void list_column_delete(list_ *list, GtkTreeViewColumn *column) {
	int liststore_remove_col_nr = -1;
	GList *columns = NULL;
	GType *types = NULL;
	GtkListStore *liststore_old = NULL;
	int i;

	if (column == NULL) {
		return;
	}
	
	liststore_remove_col_nr = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "col_nr"));

	/* Treeview: Remove column and update other columns to compensate for 
	 * shifting in liststore */
	columns = gtk_tree_view_get_columns(list->treeview);
	while (columns != NULL) {
		int col_nr;

		col_nr = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(columns->data), "col_nr"));

		if (col_nr > liststore_remove_col_nr) { /* Shift columns to left */
			GList *col_renderers = NULL;

			col_renderers = gtk_tree_view_column_get_cell_renderers(columns->data);
			gtk_tree_view_column_set_attributes(columns->data, col_renderers->data, "text", col_nr - 1, NULL);
			gtk_tree_view_column_set_sort_column_id(columns->data, col_nr - 1);
				
			g_object_set_data(G_OBJECT(columns->data), "col_nr", GUINT_TO_POINTER(col_nr -1));
			
		}
		columns = columns->next;
	}
	
	gtk_tree_view_remove_column(list->treeview, column);
		
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
		list->liststore = gtk_list_store_newv(list->nr_of_cols - 1, types);
		free(types);
		
		if (liststore_old != NULL) {
			gchar *row_data;
			GtkTreeIter iter_old;
			GtkTreeIter iter;
			
			if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore_old), &iter_old)) {
				do {
					int col_counter = 0;

					gtk_list_store_append(list->liststore, &iter);
					
					/* Copy old liststore to new liststore */
					for (i = 0; i < list->nr_of_cols; i++) {
						if (i != liststore_remove_col_nr) {
							gtk_tree_model_get(GTK_TREE_MODEL(liststore_old), &iter_old, i, &row_data, -1);
							gtk_list_store_set(list->liststore, &iter, col_counter, row_data, -1);
							col_counter++;
						}
					}
				} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore_old), &iter_old));

				/* Clean up old liststore */
				gtk_list_store_clear(liststore_old);
			}
		}
		
		gtk_tree_view_set_model(list->treeview, GTK_TREE_MODEL(list->liststore));
	}

	list->nr_of_cols--;
	list->modified = TRUE;

}

void list_row_add_empty(list_ *list) {
	GtkTreeIter treeiter;
	int i;

	if (list->liststore == NULL) {
		return;
	}
	
	gtk_list_store_append(list->liststore, &treeiter);
	for (i = 0; i < list->nr_of_cols; i++) {
		gtk_list_store_set(list->liststore, &treeiter, i, "", -1);
	}

	list->nr_of_rows++;
	list->modified = TRUE;
	
}

void list_row_add(list_ *list, int nr_of_cols, char *values[]) {
	GtkTreeIter treeiter;
	int i;

	gtk_list_store_append(list->liststore, &treeiter);
	for (i = 0; i < nr_of_cols; i++) {
		if (values[i] != NULL) {
			gtk_list_store_set(list->liststore, &treeiter, i, values[i], -1);
		} else {
			gtk_list_store_set(list->liststore, &treeiter, i, "", -1);
		}
	}

	list->nr_of_rows++;
	list->modified = TRUE;
}

void list_title_set(char *title) {
	char *title_markup, *title_win;
	
	if (list->title != NULL) {
		free(list->title);
	}

	list->title = strdup(title);
	
	title_markup = malloc(sizeof(char) * (18 + strlen(list->title) + 1));
	sprintf(title_markup, "<big><b>%s</b></big>", list->title);
	gtk_label_set_markup(GTK_LABEL(lbl_listtitle), title_markup);
	free(title_markup);

	title_win = malloc(sizeof(char) * (13 + strlen(list->title) + 1));
	sprintf(title_win, "%s - Listpatron", list->title);
	gtk_window_set_title(GTK_WINDOW(win_main), title_win);
	free(title_win);

}

list_ *list_create(void) {
	list_ *list = NULL;
	GtkTreeSelection *treeselection;
	list = malloc(sizeof(list_)); /* FIXME: Not freed */

	list->version     = strdup("0.1");      /* FIXME: Not freed */
	list->title       = strdup("Untitled");
	list->author      = strdup("");         /* FIXME: Not freed */
	list->description = strdup("");         /* FIXME: Not freed */
	list->keywords    = strdup("");         /* FIXME: Not freed */
	list->filename    = NULL;
	list->treeview    = NULL;
	list->liststore   = NULL;
	list->nr_of_cols  = 0;
	list->nr_of_rows  = 0;

	list->treeview  = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_signal_connect(
			GTK_OBJECT(list->treeview),
			"cursor-changed",
			G_CALLBACK(ui_treeview_cursor_changed_cb),
			NULL);
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(list->treeview), 1);
	treeselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list->treeview));
	gtk_tree_selection_set_mode(treeselection, GTK_SELECTION_SINGLE);
	
	list->modified = FALSE;

	return (list);
}

int list_import_csv(list_ *list, char *filename, char delimiter) {
	FILE *f = NULL;
	char buf[4096];
	int i = 0, failed_rows = 0;
	
	if (!(f = fopen(filename, "r"))) {
		return (-1);
	}

	/* Create columns */
	fgets(buf, 4096, f);
	for (i = 0; i < strlen(buf); i++) {
		if (buf[i] == delimiter || i == (strlen(buf) - 1)) {
			list_column_add(list, "Column");
		}
	}

	if (list->nr_of_cols < 1) {
		return (-1); /* Not a valid CSV */
	}

	fseek(f, 0, SEEK_SET);
	
	while (fgets(buf, 4096, f)) {
		char *field_start = NULL;
		char **rowdata = malloc(sizeof(char *) * list->nr_of_cols);
		int col = 0, l = 0;
		
		l = strlen(buf);
		field_start = &(buf[0]);
		for (i = 0; i < l; i++) {
			if ((buf[i] == delimiter || i == (l-1)) && col < list->nr_of_cols) {
				int read, write;
				GError *error = NULL;
				buf[i] = '\0';
				rowdata[col] = g_locale_to_utf8(
						field_start,
						strlen(field_start),
						&read,
						&write,
						&error);
				if (error != NULL) {
					debug_msg(DBG_WARN, __FILE__, __LINE__, "Error %i: Couldn't convert '%s' to UTF8.", error->code, field_start);
				}
						
				field_start = &(buf[i+1]);
				col++;
			}
		}
		
		if (col != list->nr_of_cols) {
			failed_rows++;
			debug_msg(DBG_WARN, __FILE__, __LINE__, "Invalid row in character seperated file '%s'.", filename);
		} else {
			list_row_add(list, list->nr_of_cols, rowdata);
		}
		free(rowdata);
	}
	
	fclose(f);

	list->modified = FALSE;

	return (failed_rows);
}

int list_export_ps(list_ *list, char *filename, int orientation) {
	GtkTreeIter iter;
	gchar *row_data;
	GList *columns = NULL, *column_iter = NULL;
	int i;

	FILE *f;
	int page_height, page_width, page_top;
	int margin_top, margin_left, margin_bottom;
	int font_size, line_spacing;
	int pos_y, pos_x;
	char *orientation_str;
	
	switch (orientation) {
		case ORIENT_PORTRAIT:
			orientation_str = strdup("Portrait");
			page_height = 842;
			page_width  = 585;
			break;
		case ORIENT_LANDSCAPE:
			orientation_str = strdup("Landscape");
			page_height = 585;
			page_width  = 842;
			break;
		default:
			orientation_str = strdup("Unknown");
			gtk_error_dialog("Unknown orientation");
			break;
	}
	
	page_top = 842;
	
	margin_top = 0;
	margin_left = 0;
	margin_bottom = 0;
	font_size = 8;
	line_spacing = 0;
	
	pos_y = page_top - margin_top;
	pos_x = 0 + margin_left;
	
	if (!(f = fopen(filename, "w"))) {
		gtk_error_dialog("Can't open file %s for writing", filename);
		return (-1);
	}
//<</Orientation 0>>setpagedevice\n
	
	/* Header */
	fprintf(f, "\
%%!PS-Adobe-2.1\n\
%%%%Orientation: %s\n\
%%%%DocumentPaperSizes: a4\n\
%%%%EndComments\n\
<</Orientation %i>>setpagedevice\n\
/Times-Roman findfont %i scalefont setfont\n\
/vpos %i def\n",
	orientation_str,
	orientation,
	font_size,
	page_top - margin_top - font_size);
	
	/* Functions : newline */
	fprintf(f, "\
/newline {\n\
/vpos vpos %i sub def\n\
%i vpos moveto\n\
} def\n", 
	font_size + line_spacing,
	margin_left);

	/* Functions : newpage */
	fprintf(f, "\
/newpage {\n\
/vpos %i def\n\
showpage\n\
%i vpos moveto\n\
} def\n",
	page_top - margin_top - font_size,
	margin_left);
	
	fprintf(f, "\
newpath\n\
%i vpos moveto\n\
", margin_left);

	columns = gtk_tree_view_get_columns(list->treeview);
	column_iter = columns;
	while (column_iter) {
		column_iter = column_iter->next;
	}

	gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter);
	do {
		for (i = 0; i < list->nr_of_cols; i++) {
			gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			if (i == 1) {
				fprintf(f, "(%s) show\n", row_data);
				pos_y -= (font_size + line_spacing);
				if (pos_y < ((page_top - page_height) + (margin_bottom + font_size))) {
					pos_y = page_top - margin_top - font_size;
					fprintf(f, "newpage\n");
				} else {
					fprintf(f, "newline\n");
				}
			}
			free(row_data);
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));
	fprintf(f, "showpage\n");

	fclose(f);

	free(orientation_str);
	
	return (0);
}

int list_export_html(list_ *list, char *filename) {
	char *row_colors[] = {"class=\"row_even\"", "class=\"row_odd\""};
	FILE *f;
	GList *columns = NULL, *column_iter = NULL;
	GtkTreeIter iter;
	gchar *row_data;
	int row_even = 0;
	int i;

	if ((f = fopen(filename, "w")) == FALSE) {
		return (-1);
	}

	fprintf(f, "\
<html>\n\
<head>\n\
	<style>\n\
		body { font-family: helvetica; font-size: 10px; }\n\
		th { text-align: left; background-color: #000000; color: #FFFFFF; }\n\
		tr.row_even { background-color: #F0F0F0; }\n\
		tr.row_odd { background-color: #FFFFFF; }\n\
	</style>\n\
</head>\n\
<body>\n\
	<table>\n\
		<tr>");
	
	/* Save column headers */
	columns = gtk_tree_view_get_columns(list->treeview);
	column_iter = columns;
	while (column_iter) {
		fprintf(f, "\t\t\t\t<th>%s</th>\n", (char *)GTK_TREE_VIEW_COLUMN(column_iter->data)->title);
		column_iter = column_iter->next;
	}
	
	fprintf(f, "\
		</tr>\n");

	/* Save row data */
	row_even = 1;
	gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter);
	do {
		fprintf(f, "\t\t\t<tr %s>\n", row_colors[row_even]);
		for (i = 0; i < list->nr_of_cols; i++) {
			gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			fprintf(f, "\t\t\t\t<td>%s</td>\n", row_data);
			free(row_data);
		}
		fprintf(f, "\t\t\t</tr>\n");
		row_even ^= 1;
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));
	
	fprintf(f, "\
	</body>\n\
</html>\n");

	fclose(f);

	return (0);
}

int list_load_filters(list_ *list, xmlNodeSetPtr node_filters) {
	if (opt_verbose) {
		printf("Filters are not implemented yet. If you want to help out, take a look at file '%s', line %i\n", __FILE__, __LINE__);
	}
	return (0);
}
int list_load_sorts(list_ *list, xmlNodeSetPtr node_sorts) {
	if (opt_verbose) {
		printf("Sorts are not implemented yet. If you want to help out, take a look at file '%s', line %i\n", __FILE__, __LINE__);
	}
	return (0);
}
int list_load_reports(list_ *list, xmlNodeSetPtr node_reports) {
	if (opt_verbose) {
		printf("Reports are not implemented yet. If you want to help out, take a look at file '%s', line %i\n", __FILE__, __LINE__);
	}
	return (0);
}

void list_load_header(list_ *list, xmlNodeSetPtr nodeset_header) {
	int i;

	for (i = 0; i < nodeset_header->nodeNr; i++) {
		if (nodeset_header->nodeTab[i]->children) {
			list_column_add(list, nodeset_header->nodeTab[i]->children->content);
		} else {
			list_column_add(list, "");
		}
	}

	list->nr_of_cols = nodeset_header->nodeNr;
}

void list_load_rows(list_ *list, xmlNodeSetPtr nodeset_rows) {
	int i;
	xmlNodePtr node_iter;
	char **rowdata;
	
	for (i = 0; i < nodeset_rows->nodeNr; i++) {
		int j = 0;
		
		node_iter = nodeset_rows->nodeTab[i]->children;
		rowdata = malloc(sizeof(char *) * list->nr_of_cols);
		
		while (node_iter != NULL) {
			if (node_iter->type == XML_ELEMENT_NODE) {
				if (node_iter->children != NULL) {
					rowdata[j] = node_iter->children->content;
				} else {
					rowdata[j] = NULL;
				}
				j++;
			}
			node_iter = node_iter->next;
		}

		list_row_add(list, list->nr_of_cols, rowdata);
		free(rowdata);
	}
}

int list_load(list_ *list, char *filename) {
	xmlDocPtr doc;
	int pos_x, pos_y, dim_width, dim_height;
	xmlNodeSetPtr nodeset = NULL;
	xmlValidCtxtPtr valid_ctxt = NULL;
	int read_options = 0; 
	
	if (opt_verbose) {
		read_options = XML_PARSE_DTDVALID;
	}
	
	if ((doc = xmlReadFile(filename, NULL, read_options | XML_PARSE_NOCDATA)) == NULL) {
		return (-1); /* Couldn't open file */
	}
	
	valid_ctxt = xmlNewValidCtxt();
	if (!xmlValidateDocument(valid_ctxt, doc)) {
		return (-2); /* Invalid Listpatron file */
	}

	if (list->version != NULL) { free(list->version); }
	if (list->author != NULL) { free(list->author); }
	if (list->description != NULL) { free(list->description); }
	if (list->keywords != NULL) { free(list->keywords); }
			
	list_title_set(xml_get_element_content(doc, "/list/info/title"));
	list->version = strdup(xml_get_element_content(doc, "/list/info/version"));
	list->author = strdup(xml_get_element_content(doc, "/list/info/author"));
	list->description = strdup(xml_get_element_content(doc, "/list/info/description"));
	list->keywords = strdup(xml_get_element_content(doc, "/list/info/keywords"));

	pos_x = atoi(xml_get_element_content(doc, "/list/state/window/position/x"));
	pos_y = atoi(xml_get_element_content(doc, "/list/state/window/position/y"));
	dim_width = atoi(xml_get_element_content(doc, "/list/state/window/dimensions/width"));
	dim_height = atoi(xml_get_element_content(doc, "/list/state/window/dimensions/height"));

	if ((nodeset = xml_get_nodeset(doc, "/list/filters/filter")) != NULL) {
		list_load_filters(list, nodeset);
	}
	if ((nodeset = xml_get_nodeset(doc, "/list/sorts/sort")) != NULL) {
		list_load_sorts(list, nodeset);
	}
	
	if ((nodeset = xml_get_nodeset(doc, "/list/reports/report")) != NULL) {
		list_load_reports(list, nodeset);
	}
	
	if ((nodeset = xml_get_nodeset(doc, "/list/header/columnname")) != NULL) {
		list_load_header(list, nodeset);
	}
	
	if ((nodeset = xml_get_nodeset(doc, "/list/rows/row")) != NULL) {
		list_load_rows(list, nodeset);
	}

	gtk_window_move(GTK_WINDOW(win_main), pos_x, pos_y);
	gtk_window_resize(GTK_WINDOW(win_main), dim_width, dim_height);

	list->filename = strdup(filename); /* FIXME: Not freed */
	list->modified = FALSE;

	return (0);
}

int list_save(list_ *list, char *filename) {
	xmlDocPtr doc;
	xmlNodePtr 
		node_root, 
		node_info,
		node_state,
		node_window,
		node_position,
		node_dimensions,
		node_header,
		node_rows,
		node_row;
	GtkTreeIter iter;
	GList *columns = NULL, *column_iter = NULL;
	int 
		pos_x, 
		pos_y, 
		dim_width, 
		dim_height;
	gchar *row_data;
	int i;
	
	gtk_window_get_size(GTK_WINDOW(win_main), &dim_width, &dim_height);
	gtk_window_get_position(GTK_WINDOW(win_main), &pos_x, &pos_y);

	/* Create XML document */
	doc = xmlNewDoc("1.0");
	xmlCreateIntSubset(doc, "list", NULL, "data/listpatron.dtd");

	node_root = xmlNewDocNode(doc, NULL, (const xmlChar *)"list", NULL);
	xmlDocSetRootElement(doc, node_root);

	/* Save application info <info> */
	node_info = xmlNewChild(node_root, NULL, (const xmlChar *)"info", NULL);
	xml_add_element_content(node_info, "version", "%s", list->version);
	xml_add_element_content(node_info, "title", "%s", list->title);
	xml_add_element_content(node_info, "author", "%s", list->author);
	xml_add_element_content(node_info, "description", "%s", list->description);
	xml_add_element_content(node_info, "keywords", "%s", list->keywords);

	/* Save application state <state>*/
	node_state = xmlNewChild(node_root, NULL, (const xmlChar *)"state", NULL);
	node_window = xmlNewChild(node_state, NULL, (const xmlChar *)"window", NULL);
	node_position = xmlNewChild(node_window, NULL, (const xmlChar *)"position", NULL);
	xml_add_element_content(node_position, "x", "%i", pos_x);
	xml_add_element_content(node_position, "y", "%i", pos_y);
	node_dimensions = xmlNewChild(node_window, NULL, (const xmlChar *)"dimensions", NULL);
	xml_add_element_content(node_dimensions, "width", "%i", dim_width);
	xml_add_element_content(node_dimensions, "height", "%i", dim_height);
	
	/* Save filters */
	/* NOT IMPLEMENTED YET */

	/* Save sorts */
	/* NOT IMPLEMENTED YET */

	/* Save reports */
	/* NOT IMPLEMENTED YET */

	/* Save column header information <header> */
	node_header = xmlNewChild(node_root, NULL, (const xmlChar *)"header", NULL);
	
	columns = gtk_tree_view_get_columns(list->treeview);
	column_iter = columns;
	while (column_iter) {
		xml_add_element_content(node_header, "columnname", "%s", (char *)GTK_TREE_VIEW_COLUMN(column_iter->data)->title);
		column_iter = column_iter->next;
	}

	/* Save row data <rows> */
	node_rows = xmlNewChild(node_root, NULL, (const xmlChar *)"rows", NULL);

	gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter);
	do {
		node_row = xmlNewChild(node_rows, NULL, (const xmlChar *)"row", NULL);
		for (i = 0; i < list->nr_of_cols; i++) {
			gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			xml_add_element_content(node_row, "column", "%s", row_data);
			free(row_data);
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));

	xmlSaveFormatFileEnc(filename, doc, "ISO-8859-1", 1);
	
	list->filename = strdup(filename); /* FIXME: Not freed */
	list->modified = FALSE;

	return (0);
}

/****************************************************************************
 * Callbacks 
 ****************************************************************************/
/* Tree and list */
void ui_treeview_cursor_changed_cb(GtkTreeView *tv, gpointer user_data) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	char *path_str;
	int col, row;

	gtk_tree_view_get_cursor(list->treeview, &path, &column);
	path_str = gtk_tree_path_to_string(path);
	row = atoi(path_str);

	col = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "col_nr"));

	gtk_statusbar_msg("Row %i, Column %i", row+1, col+1);
}

/* File open */
void ui_file_open_btn_ok_cb(GtkWidget *win, GtkFileSelection *fs) {
	char *filename = NULL;
	char *sb_message = NULL;
	int err_nr;
	
	filename = (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));
	
	if ((err_nr = list_load(list, filename)) != 0) {
		switch (err_nr) {
			case -1: gtk_error_dialog("Couldn't open file '%s'.", filename); break;
			case -2: gtk_error_dialog("Invalid listpatron file '%s'.", filename); break;
			default: gtk_error_dialog("Unknown error while opening file '%s'.", filename); break;	
		}
	} else {
		gtk_statusbar_msg("File '%s' loaded.", filename);
	}
}

void ui_menu_file_open_cb(void) {
	GtkWidget *win_file_open;

	win_file_open = gtk_file_selection_new("Open file");
	/* FIXME: Use gtk_dialog_add_buttons and gtk_dialog_run */
    g_signal_connect(
			G_OBJECT(GTK_FILE_SELECTION(win_file_open)->ok_button),
			"clicked", 
			G_CALLBACK(ui_file_open_btn_ok_cb), 
			(gpointer) win_file_open);
    g_signal_connect_swapped(
			G_OBJECT(GTK_FILE_SELECTION(win_file_open)->ok_button),
			"clicked", 
			G_CALLBACK(gtk_widget_destroy), 
			G_OBJECT(win_file_open));
    g_signal_connect_swapped(
			G_OBJECT(GTK_FILE_SELECTION(win_file_open)->cancel_button),
			"clicked", 
			G_CALLBACK(gtk_widget_destroy), 
			G_OBJECT(win_file_open));
	
	gtk_widget_show(win_file_open);
}

/* File import */
void ui_file_import_csv_btn_ok_cb(GtkWidget *win, GtkFileSelection *fs) {
	char *filename = NULL;
	char *delimiter_string = NULL;
	int rows = -1;
	
	filename = (char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs)); /* FIXME: Should this be freed? */
	
	delimiter_string = gtk_input_dialog("Enter a single character which delimits the fields in the file", ",");
	if (delimiter_string != NULL) {
		if (list_import_csv(list, filename, ',') == -1) {
			gtk_error_dialog("Not a correct Comma Separated file '%s'", filename);
		} else {
			gtk_statusbar_msg("File %s imported. %i rows read.", filename, list->nr_of_rows, rows);
		}

		free(delimiter_string);
	}
}

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
void ui_file_save_btn_ok_cb(GtkWidget *win, GtkFileSelection *fs) {
	char *filename = NULL;
	
	filename =(char *)gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));
	
	list_save(list, filename);

	gtk_statusbar_msg("File '%s' saved.", filename);
}

void ui_menu_file_save_cb(void) {
	GtkWidget *dia_file_save;
	int response;

	if (list->filename == NULL) {
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
		
		response = gtk_dialog_run(GTK_DIALOG(dia_file_save));

		if (response == GTK_RESPONSE_ACCEPT) {
			list->filename = strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dia_file_save)));
			gtk_widget_destroy(dia_file_save);
		} else {
			gtk_widget_destroy(dia_file_save);
			return;
		}
	}

	list_save(list, list->filename);
}

/* File save as... */
void ui_menu_file_save_as_cb(void) {
	char *old_filename;

	old_filename = list->filename;
	list->filename = NULL;
	ui_menu_file_save_cb();
	list->filename = old_filename;
}

void ui_menu_file_quit_cb(void) {
	if (list->modified == TRUE) {
		GtkWidget *dia_modified;
		GtkWidget *lbl_modified;
		int result;
		
		dia_modified = gtk_dialog_new_with_buttons(
				"Save changes?",
				GTK_WINDOW(win_main),
				GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_STOCK_YES, GTK_RESPONSE_YES,
				GTK_STOCK_NO, GTK_RESPONSE_NO,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				NULL);
		
		lbl_modified = gtk_label_new("List was modified. Do you want to save?");

		gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_modified)->vbox), GTK_WIDGET(lbl_modified), FALSE, TRUE, 5);
		
		gtk_widget_show_all(dia_modified);

		result = gtk_dialog_run(GTK_DIALOG(dia_modified));

		gtk_widget_destroy(dia_modified);

		switch (result) {
			case GTK_RESPONSE_YES: 
				ui_menu_file_save_cb();
				break;
			case GTK_RESPONSE_CANCEL:
				return;
				break;
			default:
				break;
		}
	}
	gtk_main_quit();
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
	
	gtk_tree_view_get_cursor(list->treeview, &path, &column);
	
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
	
	gtk_tree_view_get_cursor(list->treeview, &path, &column);

	list_column_delete(list, column);
}

/* Row menu options */
void ui_menu_row_add_cb(void) {
	list_row_add_empty(list);
}

void ui_menu_row_delete_cb(void) {
	GtkTreePath *path;
	GtkTreeViewColumn *column; /* Unused */
	GtkTreeIter iter;

	gtk_tree_view_get_cursor(list->treeview, &path, &column);
	
	if (path != NULL) { /* Row selected? */
		if (!gtk_tree_model_get_iter(GTK_TREE_MODEL(list->liststore), &iter, path)) {
			return;
		}
		
		gtk_list_store_remove(GTK_LIST_STORE(list->liststore), &iter);
	}
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
		
		list_row_add(list, _TEST_COLS, col_vals);
		
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
		
		list_row_add(list, _TEST_COLS, col_vals);
		
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
	gtk_tree_view_get_cursor(list->treeview, &path, &column);
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
	
	eventbox = gtk_event_box_new();
	lbl_listtitle = gtk_label_new(NULL);

	gtk_container_add(GTK_CONTAINER(eventbox), lbl_listtitle);
	
	g_signal_connect(
			eventbox, 
			"button-press-event", 
			(GCallback) ui_listtitle_click_cb, 
			NULL);
	
	list_title_set(list->title);
	
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
	if (optind < argc)
	{
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
	gtk_container_add(GTK_CONTAINER(win_scroll), GTK_WIDGET(list->treeview));
	
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
