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
#ifndef LIST_H
#define LIST_H

/* List find options */
#define FIND_MATCHCASE 1
#define FIND_MATCHFULL 2

/* Export Printable (PS) options */
#define ORIENT_PORTRAIT 0
#define ORIENT_LANDSCAPE 1

typedef struct sort_col_ {
	char *col_name;
	int col_nr;
	int sort_order;
} sort_col_;

typedef struct sort_ {
	char *name;
	GArray *columns; /* (sort_col_ *) */
} sort_;

typedef struct rule_ {
	char *name;
	void *data; /* Pointer to sort_, filter_ or report_ */
} rule_;

typedef struct list_ {
	char *filename;

	/* Info */
	char *version;
	char *title;
	char *author;
	char *description;
	char *keywords;

	GtkListStore *liststore; /* Row information */
	GArray *columns; /* Column titles (char *) */
	
	sort_col_ sort_single; /* Sorting on a single column (when user presses a col header */
	GArray *sort_active; /* Currenly active user-defined sorting rule (sort_col_ *) */
	GArray *sorts;       /* Array of all user-defined sorting rules (sort_ *) */
	
	int nr_of_cols;
	int nr_of_rows;
	int modified;

	int last_occ_col;
	int last_occ_row;
} list_;

typedef struct find_ {/*  Move to listpatron.c */
	GtkWidget *ent_needle;
	int matchcase;
	int matchfull;
} find_;

void list_sort_dump_rules(void);
sort_ *list_sort_getrule(char *name);
void list_sort_add(char *old_name, char *name, GArray *columns);
void list_sort_remove(char *name);
gint list_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);
void list_column_add(list_ *list, char *title);
void list_column_delete(list_ *list, GtkTreeViewColumn *column);
void list_column_rename(int col_nr, char *title);
void list_row_add_empty(list_ *list);
void list_row_add(list_ *list, char *values[]);
void list_row_delete(list_ *list, GList *row_refs);
list_ *list_create(void);
void list_title_set(char *title);
void list_filename_set(list_ *list, char *filename);
void list_clear(void);
int list_import_csv(list_ *list, char *filename, char delimiter);
int list_export_csv(list_ *list, char *filename, char delimiter);
int list_export_ps(list_ *list, char *filename, int orientation);
int list_export_html(list_ *list, char *filename);
int list_load(list_ *list, char *filename);
int list_save(list_ *list, char *filename);
int list_save_check(list_ *list);
int list_find(list_ *list, char *needle, int options, int *occ_row, int *occ_col);

#endif
