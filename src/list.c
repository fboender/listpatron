/****************************************************************************
 *
 * ListPatron - list
 *
 * Information list routines
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

/* TODO: 
 *  - List uses column numbers for identification of columns; however, this
 *    causes many problems with three-tier (separation of data and view) and
 *    removal of columns 
 */
#include <string.h>
#include <assert.h>

#include <gtk/gtk.h>
#include <glib-object.h>

#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "list.h"
#include "debug.h"
#include "listpatron.h"
#include "libxmlext.h"

list_ *list = NULL;
extern GtkTreeView *treeview;
extern GtkWidget *lbl_listtitle;
extern GtkWidget *win_main;
extern int opt_help;
extern int opt_batch;
extern int opt_verbose;
extern int opt_version;

void list_sort_dump_rules(list_ *list) {
	int i;
	int i_col;

	for (i = 0; i < list->sorts->len; i++) {
		sort_ *sort;

		sort = g_array_index(list->sorts, sort_ *, i);
		printf("sort->name=\"%s\"\n", sort->name);

		for (i_col = 0; i_col < sort->columns->len; i_col++) {
			sort_col_ *sort_col = g_array_index(sort->columns, sort_col_ *, i_col);
			printf("\tcolumns[%i]-> col_nr=\"%i\", sort_order=\"%i\", col_name=\"%s\", \n", 
					i_col, 
					sort_col->col_nr,
					sort_col->sort_order,
					sort_col->col_name);
		}
	}
	printf("------------------------------------------------\n");
}

sort_ *list_sort_getrule(list_ *list, char *name) {
	int i;

	assert(name != NULL);

	for (i = 0; i < list->sorts->len; i++) {
		sort_ *sort;

		sort = g_array_index(list->sorts, sort_ *, i); 
		
		if (strcmp(sort->name, name) == 0) {
			return(sort);
		}
	}

	return(NULL);
}

void list_sort_add(list_ *list, char *old_name, char *name, GArray *columns) {
	int i;
	sort_ *sort = NULL;

	assert(name != NULL);
	assert(columns != NULL);
	
	if (old_name != NULL) {
		/* Find old sorting rule and deep free it */
		for (i = 0; i < list->sorts->len; i++) {
			sort_ *dup_sort;

			dup_sort = g_array_index(list->sorts, sort_ *, i);
			if (strcmp(dup_sort->name, old_name) == 0) {
				int i_col;
				
				free(dup_sort->name);
				
				for (i_col = 0; i_col < dup_sort->columns->len; i_col++) {
					sort_col_ *sort_col = g_array_index(dup_sort->columns, sort_col_ *, i_col);

					free(sort_col->col_name);
					free(sort_col);
				}
				g_array_free(dup_sort->columns, TRUE);
				sort = dup_sort;
			}
		}
	}
	
	if (sort == NULL) {
		/* New rule or old rule not found */
		sort = malloc(sizeof(sort_));
		g_array_append_val(list->sorts, sort);
	}

	sort->name = strdup(name);
	sort->columns = columns;
}

void list_sort_remove(list_ *list, char *name) {
	int i;

	for (i = 0; i < list->sorts->len; i++) {
		sort_ *sort;

		sort = g_array_index(list->sorts, sort_ *, i);
		if (strcmp(sort->name, name) == 0) {
			free(sort->name);
			g_array_remove_index(list->sorts, i);
			break;
		}
	}
}

/* FIXME: This routine can and should be optimized. */
gint list_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data) {
	list_ *list;
	char *row_data_a;
	char *row_data_b;
	int i;

	assert(user_data != NULL);
	
	list = user_data;
	
	for (i = 0; i < list->sort_active->len; i++) {
		sort_col_ *sort_col;
		int cmp_rslt;
		
		sort_col = g_array_index(list->sort_active, sort_col_ *, i);
		
		gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), a, sort_col->col_nr, &row_data_a, -1);
		gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), b, sort_col->col_nr, &row_data_b, -1);
		
		/* On row insert the row_data might be NULL. Perhaps the insert
		 * signal is fired before the data is in the list?
		 *
		 * It seems this only happens when inserting an empty row. Perhaps
		 * list_row_add_empty() is at fault? */
		if (row_data_a == NULL || row_data_b == NULL) {
			return(0);
		}

		if ((cmp_rslt = strcmp(row_data_a, row_data_b)) != 0) {
			if (sort_col->sort_order == GTK_SORT_ASCENDING) {
				return(cmp_rslt);
			} else {
				return(cmp_rslt > 0 ? -1 : 1);
			}
		}
	}

	return (0);
}

/* Remove a column from ALL sorting rules */
void list_sorts_remove_column(int col_nr) {
	int i;

	for (i = 0; i < list->sorts->len; i++) {
		sort_ *sort;
		int j;
		int col_nr_remove = -1;
		
		sort = g_array_index(list->sorts, sort_ *, i);

		for (j = 0; j < sort->columns->len; j++) {
			sort_col_ *sort_col;

			sort_col = g_array_index(sort->columns, sort_col_ *, j);

			if (sort_col->col_nr == col_nr) {
				col_nr_remove = j;
			}

			/* Shift columns to compensate for removed col */
			if (sort_col->col_nr > col_nr && col_nr_remove != -1) {
				sort_col->col_nr--;
			}
		}

		if (col_nr_remove != -1) {
			g_array_remove_index(sort->columns, col_nr_remove);
		}
	}
}

void list_column_add(list_ *list, char *title) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GType *types = NULL;
	GtkListStore *liststore_old = NULL;
	gchar *title_mem; /* For g_array_append */
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

	title_mem = strdup(title); /* FIXME: unfreed */
	g_array_append_val(list->columns, title_mem);

	/* FIXME: This doesn't belong here. It should be handled in listpatron.c */
	g_signal_connect(renderer, "edited", (GCallback) ui_cell_edited_cb, NULL);

	gtk_tree_view_column_set_resizable(GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_reorderable(GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_clickable(GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_sort_column_id(GTK_TREE_VIEW_COLUMN(col), list->nr_of_cols);

	gtk_tree_view_append_column(treeview, col);

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
					free(row_data);
				}
				gtk_list_store_set(list->liststore, &iter, list->nr_of_cols, "", -1);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore_old), &iter_old));

			/* Clean up old liststore */
			gtk_list_store_clear(liststore_old);
			
		}
	}

	gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(list->liststore));
	
	list->nr_of_cols++;
	list->modified = TRUE;
	
}

void list_column_delete(list_ *list, GtkTreeViewColumn *column) {
	int liststore_remove_col_nr = -1;
	GList *columns = NULL, *column_iter;
	GType *types = NULL;
	GtkListStore *liststore_old = NULL;
	int i;
	char *removed_col;

	if (column == NULL) {
		return;
	}
	
	liststore_remove_col_nr = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column), "col_nr"));

	removed_col = g_array_index(list->columns, char*, liststore_remove_col_nr);
	list->columns = g_array_remove_index(list->columns, liststore_remove_col_nr);
	free(removed_col);

	/* Treeview: Remove column and update other columns to compensate for 
	 * shifting in liststore */
	columns = gtk_tree_view_get_columns(treeview);
	column_iter = columns;
	while (column_iter != NULL) {
		int col_nr;

		col_nr = GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(column_iter->data), "col_nr"));

		if (col_nr > liststore_remove_col_nr) { /* Shift columns to left */
			GList *col_renderers = NULL;

			col_renderers = gtk_tree_view_column_get_cell_renderers(column_iter->data);
			gtk_tree_view_column_set_attributes(column_iter->data, col_renderers->data, "text", col_nr - 1, NULL);
			gtk_tree_view_column_set_clickable(GTK_TREE_VIEW_COLUMN(column_iter->data), TRUE);
			gtk_tree_view_column_set_sort_column_id(column_iter->data, col_nr - 1);
				
			g_object_set_data(G_OBJECT(column_iter->data), "col_nr", GUINT_TO_POINTER(col_nr -1));

			g_list_free(col_renderers);
			
		}

		column_iter = column_iter->next;
	}
	g_list_free(columns);
	
	gtk_tree_view_remove_column(treeview, column);
		
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
							free(row_data);
							col_counter++;
						}
					}
				} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore_old), &iter_old));

				/* Clean up old liststore */
				gtk_list_store_clear(liststore_old);
			}
		}
		
		gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(list->liststore));
	}

	list->nr_of_cols--;

	list_sorts_remove_column(liststore_remove_col_nr);
		
	list->modified = TRUE;

}

void list_column_rename(int col_nr, char *title) {
	char *title_index = NULL;

	/* NOTICE: I've got the idea that this routine might not be correct.
	 * We're getting the pointer stored in the array and changing the 
	 * memory allocated to it. There might be problems with the address
	 * changing and the g_array not being in sync anymore.
	 */
	title_index = g_array_index(list->columns, char *, col_nr);
	title_index = realloc(title_index, sizeof(char) * (strlen(title) + 1));
	strcpy(title_index, title);
	
	/* Do some sanity checking because of the notice above */
	{
		int pos;
		title_index = g_array_index(list->columns, char *, col_nr);
		for (pos = 0; title[pos] != '\0' && title_index[pos] == title[pos]; pos++) { ; }
		assert(title_index[pos] == '\0');
	}

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

void list_row_add(list_ *list, char *values[]) {
	GtkTreeIter treeiter;
	int i;

	if (!(list->nr_of_cols > 0)) {
		return; /* Can't add a row to an empty list */
	}

	gtk_list_store_append(list->liststore, &treeiter);
	for (i = 0; i < list->nr_of_cols; i++) {
		if (values[i] != NULL) {
			gtk_list_store_set(list->liststore, &treeiter, i, values[i], -1);
		} else {
			gtk_list_store_set(list->liststore, &treeiter, i, "", -1);
		}
	}

	list->nr_of_rows++;
	list->modified = TRUE;
}

void list_row_delete(list_ *list, GList *row_refs) {
	GList *iter = NULL;

	if (row_refs == NULL) { /* No selected rows */
		return;
	}

	iter = row_refs;
	while (iter != NULL) {
		GtkTreePath *path;

		path = gtk_tree_row_reference_get_path((GtkTreeRowReference*)iter->data);

		if (path) {
			GtkTreeIter iter;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(list->liststore), &iter, path)) {
				gtk_list_store_remove(list->liststore, &iter);
			}

			gtk_tree_path_free(path);
		}

		list->nr_of_rows--;
		list->modified = TRUE;

		iter = iter->next;
	}

	list->modified = TRUE;
}

void list_title_set(list_ *list, char *title) {
	char *title_markup, *title_win;
	
	assert (title != list->title);

	if (list->title != NULL) {
		free(list->title);
	}

	list->title = strdup(title);
	
	/* FIXME: THis is UI dependant, move to listpatron.c */
	title_markup = malloc(sizeof(char) * (18 + strlen(list->title) + 1));
	sprintf(title_markup, "<big><b>%s</b></big>", list->title);
	gtk_label_set_markup(GTK_LABEL(lbl_listtitle), title_markup);
	free(title_markup);

	title_win = malloc(sizeof(char) * (13 + strlen(list->title) + 1));
	sprintf(title_win, "%s - Listpatron", list->title);
	gtk_window_set_title(GTK_WINDOW(win_main), title_win);
	free(title_win);

	list->modified = TRUE;
}

list_ *list_create(void) {
	list_ *list = NULL;
	list = malloc(sizeof(list_)); /* FIXME: Not freed */

	list->columns      = g_array_new(FALSE, FALSE, sizeof(char*));
	list->version      = strdup("0.1");      /* FIXME: Not freed */
	list->title        = strdup("Untitled");
	list->author       = strdup("");         /* FIXME: Not freed */
	list->description  = strdup("");         /* FIXME: Not freed */
	list->keywords     = strdup("");         /* FIXME: Not freed */
	list->filename     = NULL;
	list->liststore    = NULL;
	list->nr_of_cols   = 0;
	list->nr_of_rows   = 0;
	list->sort_active  = g_array_new(FALSE, FALSE, sizeof(sort_col_ *));
	list->sorts        = g_array_new(FALSE, FALSE, sizeof(sort_ *));
	list->last_occ_row = -1;
	list->last_occ_col = -1;
	list->sort_single.col_name = NULL;
	list->sort_single.col_nr = 0;

	list->modified = FALSE;

	return (list);
}

void list_clear(list_ *list) {
	if (list != NULL) {
		GList *columns = NULL, *column_iter;
		
		/* Clear the data */
		if (list->liststore != NULL) {
			gtk_list_store_clear(list->liststore);
		}
		
		/* Throw away columns in the treeview */
		g_object_ref(treeview);
		columns = gtk_tree_view_get_columns(treeview);
		if (columns != NULL) {
			column_iter = columns;
			while (column_iter != NULL) {
				list_column_delete(list, GTK_TREE_VIEW_COLUMN(column_iter->data));
				column_iter = column_iter->next;
			}
			g_list_free(columns);
		}

		/* Deep free list structure */
		if (list->version != NULL) { free(list->version); list->version = NULL; }
		if (list->title != NULL) { free(list->title); list->title = NULL; }
		if (list->author != NULL) { free(list->author); list->author = NULL; }
		if (list->description != NULL) { free(list->description); list->description = NULL; }
		if (list->keywords != NULL) { free(list->keywords); list->keywords = NULL; }
		if (list->filename != NULL) { free(list->filename); list->filename = NULL; }

		free(list);
		list = NULL; /* FIXME: Does this reference the local or global var? */
	}
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
			list_row_add(list, rowdata);
		}
		free(rowdata);
	}
	
	fclose(f);

	list->modified = TRUE;

	return (failed_rows);
}

int list_export_csv(list_ *list, char *filename, char delimiter) {
	GtkTreeIter iter;
	gchar *row_data;
	int i;

	FILE *f;
	
	if (!(f = fopen(filename, "w"))) {
		gtk_error_dialog("Can't open file %s for writing", filename);
		return (-1);
	}

	gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter);
	do {
		for (i = 0; i < list->nr_of_cols; i++) {
			gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			fprintf(f, "%s", row_data);
			if (i < list->nr_of_cols-1) {
				fprintf(f, "%c", delimiter);
			}
			free(row_data);
		}
		fprintf(f, "\n");
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));

	fclose(f);

	return (0);
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

	columns = gtk_tree_view_get_columns(treeview);
	column_iter = columns;
	while (column_iter) {
		column_iter = column_iter->next;
	}
	g_list_free(columns);

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
	columns = gtk_tree_view_get_columns(treeview);
	column_iter = columns;
	while (column_iter) {
		fprintf(f, "\t\t\t\t<th>%s</th>\n", (char *)GTK_TREE_VIEW_COLUMN(column_iter->data)->title);
		column_iter = column_iter->next;
	}
	g_list_free(columns);

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
	/* TODO: Implement */
	return (0);
}

int list_load_sorts(list_ *list, xmlNodeSetPtr nodeset_sorts) {
	int i;
	xmlNodePtr node_iter;
	
	for (i = 0; i < nodeset_sorts->nodeNr; i++) {
		char *sort_name = NULL;
		GArray *sort_cols;

		node_iter = nodeset_sorts->nodeTab[i]->children; /* sorts/sort* */
	
		while (node_iter != NULL) {
			/* sortname */
			if (strcmp(node_iter->name, "sortname") == 0 && node_iter->children != NULL) {
				sort_name = node_iter->children->content;
			}
			/* sortcolumns */
			if (strcmp(node_iter->name, "sortcolumns") == 0) {
				xmlNodePtr node_sort_iter;

				sort_cols = g_array_new(FALSE, FALSE, sizeof(sort_col_ *));
				
				node_sort_iter = node_iter->children;
				while (node_sort_iter != NULL) {
					if (node_sort_iter->type == XML_ELEMENT_NODE && node_sort_iter->children != NULL) {
						sort_col_ *sort_col = malloc(sizeof(sort_col_));
						char *col_nr_str = NULL;

						sort_col->col_name = strdup(node_sort_iter->children->content);
						if (strcmp(xmlGetProp(node_sort_iter, "sortdirection"), "asc") == 0) {
							sort_col->sort_order = 0;
						} else {
							sort_col->sort_order = 1;
						}
						
						col_nr_str = xmlGetProp(node_sort_iter, "sortposition");
						sort_col->col_nr = atoi(col_nr_str);

						g_array_append_val(sort_cols, sort_col);
					}
					node_sort_iter = node_sort_iter->next; /* Next <columnname> */
				}

			}

			node_iter = node_iter->next; /* Next element in sorts/sort/ */
		}
		list_sort_add(list, NULL, sort_name, sort_cols);
	}

	return (0);
}

int list_load_reports(list_ *list, xmlNodeSetPtr node_reports) {
	/* TODO: Implement */
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

int list_load_rows(list_ *list, xmlNodeSetPtr nodeset_rows) {
	int i;
	xmlNodePtr node_iter;
	char **rowdata;
	int err = 0;
	
	for (i = 0; i < nodeset_rows->nodeNr; i++) {
		int j = 0;
		
		node_iter = nodeset_rows->nodeTab[i]->children;
		rowdata = malloc(sizeof(char *) * list->nr_of_cols);
		
		while (node_iter != NULL) {
			if (node_iter->type == XML_ELEMENT_NODE) {
				if (j > list->nr_of_cols) {
					break; /* Corrupt file */
				}
				if (node_iter->children != NULL) {
					rowdata[j] = node_iter->children->content;
				} else {
					rowdata[j] = NULL;
				}
				j++;
			}
			node_iter = node_iter->next;
			
		}

		if (j == list->nr_of_cols) {
			list_row_add(list, rowdata);
		} else {
			err = -3;
			if (j > list->nr_of_cols) {
				fprintf(stderr, "Invalid row (too many columns). Skipping. FILE IS PROBABLY CORRUPT.\n");
			} else {
				fprintf(stderr, "Invalid row (too few columns). Skipping. FILE IS PROBABLY CORRUPT.\n");
			}
		}

		free(rowdata);
	}

	return(err);
}

int list_load(list_ *list, char *filename) {
	xmlDocPtr doc;
	int pos_x, pos_y, dim_width, dim_height;
	xmlNodeSetPtr nodeset = NULL;
	xmlValidCtxtPtr valid_ctxt = NULL;
	int read_options = 0; 
	int err = 0;
	
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
			
	list_title_set(list, xml_get_element_content(doc, "/list/info/title"));
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
		if (list_load_rows(list, nodeset) != 0) {
			err = -3;
		}
	}

	xmlFreeDoc(doc);

	/* FIXME: Move this to listpatron.c. It's the UI's responsibility to move the window */
	gtk_window_move(GTK_WINDOW(win_main), pos_x, pos_y);
	gtk_window_resize(GTK_WINDOW(win_main), dim_width, dim_height);

	list_filename_set(list, filename);
	list->modified = FALSE;

	if (err != 0) { 
		return(err);
	} else {
		return (0);
	}
}

int list_save_sorts(list_ *list, xmlNodePtr node_sorts) {
	int i;

	for (i = 0; i < list->sorts->len; i++) {
		int i_col;
		sort_ *sort = NULL;
		xmlNodePtr 
			node_sort,
			node_sortcolumns;
			
		sort = g_array_index(list->sorts, sort_ *, i);
		
		node_sort = xmlNewChild(node_sorts, NULL, (const xmlChar *)"sort", NULL);
		xml_add_element_content(node_sort, "sortname", "%s", sort->name);
		node_sortcolumns = xmlNewChild(node_sort, NULL, (const xmlChar *)"sortcolumns", NULL);

		for (i_col = 0; i_col < sort->columns->len; i_col++) {
			char *sort_dir_str[] = {"asc", "desc"};
			sort_col_ *sort_col;
			int sort_dir;
			gchar *column_name;
			char *col_nr = malloc(sizeof(char) * 8);
			xmlNodePtr node;

			sort_col = g_array_index(sort->columns, sort_col_ *, i_col);

			column_name = g_array_index(list->columns, gchar *, sort_col->col_nr);
			sort_dir = sort_col->sort_order; /* FIXME: Sometimes sort_dir, sort_order etc. Fix everywhere. */
			sprintf(col_nr, "%i", sort_col->col_nr);

			node = xml_add_element_content(node_sortcolumns, "columnname", "%s", column_name);
			xmlNewProp(node, "sortdirection", sort_dir_str[sort_dir]); /* FIXME: DOesn't this need tree.h? */
			xmlNewProp(node, "sortposition", col_nr); 

			free (col_nr);
		}
	}

	return(0);
}

void list_filename_set(list_ *list, char *filename) {
	if (list->filename != NULL) {
		free(list->filename);
	}
	if (filename == NULL) {
		list->filename = NULL;
	} else {
		list->filename = strdup(filename);
	}
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
		node_sorts,
		node_header,
		node_rows,
		node_row;
	GtkTreeIter iter;
	int 
		pos_x, 
		pos_y, 
		dim_width, 
		dim_height;
	gchar *row_data;
	int i;
	
	assert(filename != list->filename);

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
	/* TODO: Implement */

	/* Save sorts */
	node_sorts = xmlNewChild(node_root, NULL, (const xmlChar *)"sorts", NULL);
	list_save_sorts(list, node_sorts);
	
	/* Save reports */
	/* TODO: Implement */

	/* Save column header information <header> */
	node_header = xmlNewChild(node_root, NULL, (const xmlChar *)"header", NULL);
	
	debug_msg(DBG_VERBOSE, __FILE__, __LINE__, "%s", "Column sequence:\n");
	/* FIXME: Redundant: isn't used? */
	for (i = 0; i < list->nr_of_cols; i++) {
		GtkTreeViewColumn* col;
		col = gtk_tree_view_get_column(treeview, i);
		debug_msg(DBG_VERBOSE, __FILE__, __LINE__, "  %s", col->title);
	}
	
	for (i = 0; i < list->columns->len; i++) {
		char *title;
		title = g_array_index(list->columns, char *, i);
		xml_add_element_content(node_header, "columnname", "%s", (char *)title);
	}

	/* Save row data <rows> */
	node_rows = xmlNewChild(node_root, NULL, (const xmlChar *)"rows", NULL);

	if(gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter)) {
		do {
			node_row = xmlNewChild(node_rows, NULL, (const xmlChar *)"row", NULL);
			for (i = 0; i < list->nr_of_cols; i++) {
				GtkTreeViewColumn* col;

				gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
				
				col = gtk_tree_view_get_column(treeview, i);
				xml_add_element_content(node_row, "column", "%s", row_data);
				free(row_data);
			}
		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));
	}

	if (filename == NULL) {
		xmlSaveFormatFileEnc(list->filename, doc, "ISO-8859-1", 1);
		list_filename_set(list, filename);
	} else {
		xmlSaveFormatFileEnc(filename, doc, "ISO-8859-1", 1);
	}
	xmlFreeDoc(doc);
	
	list->modified = FALSE;

	return (0);
}

/* returns  1 if the user requested the file should be saved,
 *         -1 if the user want's to stop the operation
 *          0 list doesn't need to be saved */
/* FIXME: Doesn't belong here. UI specific stuff */
int list_save_check(list_ *list) {
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
				ui_menu_file_save_cb(); /* FIXME: Doesn't belong */
				return (1);
				break;
			case GTK_RESPONSE_CANCEL:
				return (-1); /* Cancel operation */
				break;
			default:
				return (0); /* No need to save */
				break;
		}
	} else {
		return (0); /* No need to save */
	}
}

/* FIXME: Can't search up, can't restart search. can't start search at certain
 * row/cols
 */
int list_find(list_ *list, char *needle, int options, int *occ_row, int *occ_col) {
	GtkTreeIter iter;
	int i, row = 0;
	gchar *row_data;

	if (list->last_occ_row != -1 && list->last_occ_col != -1) {
		/* Resume search */
		char *path_str = malloc(sizeof(char) * 10);

		sprintf(path_str, "%i", list->last_occ_row);

		gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(list->liststore), &iter, path_str);
		row = list->last_occ_row;
		i = list->last_occ_col + 1;

		list->last_occ_col = -1;
		list->last_occ_row = -1;

		free(path_str);
	} else {
		/* Start new search */
		i = 0;
		gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter);
	}
	
	do {
		while (i < list->nr_of_cols) {
			char *result = NULL;
			GtkTreeViewColumn* col;

			gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			col = gtk_tree_view_get_column(treeview, i);

			if ((options & FIND_MATCHCASE) == FIND_MATCHCASE) {
				/* Case sensitive */

				if ((options & FIND_MATCHFULL) == FIND_MATCHFULL) {
					/* Full matches only */
					if (strcmp(row_data, needle) == 0) {
						result = row_data;
					}
				} else {
					/* Partial matches allowed */
					result = strstr(row_data, needle);
				}
			} else {
				/* Case insensitive */
				gchar *lc_row_data, *lc_needle;
				
				lc_row_data = g_ascii_strdown(row_data, -1);
				lc_needle = g_ascii_strdown(needle, -1);
				
				if ((options & FIND_MATCHFULL) == FIND_MATCHFULL) {
					/* Full matches only */
					if (strcmp(lc_row_data, lc_needle) == 0) {
						result = row_data;
					}
				} else {
					/* Partial matches allowed */
					result = strstr(lc_row_data, lc_needle);
				}

				g_free(lc_row_data);
				g_free(lc_needle);
			}

			if (result != NULL) {
				*occ_row = row;
				*occ_col = i;

				/* Save search */
				list->last_occ_row = row;
				list->last_occ_col = i;

				free (row_data);
				return(1);
			}

			free(row_data);
			i++;
		}
		
		i = 0;
		row++;
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));
	
	return(0);
}

