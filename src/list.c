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

	printf ("%i\n", liststore_remove_col_nr);
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

	assert(row_refs != NULL);

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
		iter = iter->next;
	}
}

void list_title_set(char *title) {
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

}

list_ *list_create(void) {
	list_ *list = NULL;
	list = malloc(sizeof(list_)); /* FIXME: Not freed */

	list->columns     = g_array_new(FALSE, FALSE, sizeof(char*));
	list->version     = strdup("0.1");      /* FIXME: Not freed */
	list->title       = strdup("Untitled");
	list->author      = strdup("");         /* FIXME: Not freed */
	list->description = strdup("");         /* FIXME: Not freed */
	list->keywords    = strdup("");         /* FIXME: Not freed */
	list->filename    = NULL;
	list->liststore   = NULL;
	list->nr_of_cols  = 0;
	list->nr_of_rows  = 0;
	list->last_occ_row = -1;
	list->last_occ_col = -1;

	list->modified = FALSE;

	return (list);
}

void list_clear(void) {
	if (list != NULL) {
		GList *columns = NULL, *column_iter;
		
		/* Clear the data */
		if (list->liststore != NULL) {
			gtk_list_store_clear(list->liststore);
		}
		
		/* Throw away columns in the treeview */
		g_object_ref(treeview);
		columns = gtk_tree_view_get_columns(treeview);
		column_iter = columns;
		while (column_iter != NULL) {
			list_column_delete(list, GTK_TREE_VIEW_COLUMN(column_iter->data));
			column_iter = column_iter->next;
		}
		g_list_free(columns);

		/* Deep free list structure */
		if (list->version != NULL) { free(list->version); }
		if (list->title != NULL) { free(list->title); }
		if (list->author != NULL) { free(list->author); }
		if (list->description != NULL) { free(list->description); }
		if (list->keywords != NULL) { free(list->keywords); }
		if (list->filename != NULL) { free(list->filename); }

		free(list);
		list = NULL; /* FIXME: Does this reference the local or global var? */
	} else {
		gtk_statusbar_msg("It's really empty. Don't worry"); /* FIXME: Doesn't belong here */
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

	list->modified = FALSE;

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

		list_row_add(list, rowdata);
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

	xmlFreeDoc(doc);

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
	
	debug_msg(DBG_VERBOSE, __FILE__, __LINE__, "%s", "Column sequence:\n");
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

	gtk_tree_model_get_iter_root(GTK_TREE_MODEL(list->liststore), &iter);
	do {
		node_row = xmlNewChild(node_rows, NULL, (const xmlChar *)"row", NULL);
		for (i = 0; i < 1; i++) {
			GtkTreeViewColumn* col;

			gtk_tree_model_get(GTK_TREE_MODEL(list->liststore), &iter, i, &row_data, -1);
			
			col = gtk_tree_view_get_column(treeview, i);
			xml_add_element_content(node_row, "column", "%s", row_data);
			printf("list_save: *%s*\n", row_data);
			free(row_data);
		}
		printf("\n");
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(list->liststore), &iter));

	xmlSaveFormatFileEnc(filename, doc, "ISO-8859-1", 1);
	xmlFreeDoc(doc);
	
	list->filename = strdup(filename); /* FIXME: Not freed */
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

