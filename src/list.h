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

/* Convenience function which, when called, will dump the sorting rules in
 * 'list'.
 */
void list_sort_dump_rules(list_ *list);

/* Returns the sorting rule with 'name' from 'list'.
 * If the rule couldn't be found it returns NULL 
 */
sort_ *list_sort_getrule(list_ *list, char *name);

/* Adds OR OVERWRITES a sorting rule named 'name' with the sorting columns in 
 * 'columns'. 'columns' is a GArray of sort_col_ structures. If 'old_name'
 * is NULL, a new sorting rule is added to 'list' with the name 'name'. 
 * Otherwise, the sorting rule with the name 'old_name' is removed and 
 * replaced with a new sorting rule with the name 'name'
 */
void list_sort_add(list_ *list, char *old_name, char *name, GArray *columns);

/* Removes the sorting rule named 'name' from 'list' */
void list_sort_remove(list_ *list, char *name);

/* Compares the data in 'a' with 'b' (using information from 'list') and
 * returns 0 if they are equal, > 0 if 'a' should appear before 'b' or
 * < 0 if 'b' should appear before 'a'
 */
gint list_sort_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);

/* Adds a column with the title 'title' to 'list' */
void list_column_add(list_ *list, char *title);

/* Deletes the column identified by 'column' from 'list' */
void list_column_delete(list_ *list, GtkTreeViewColumn *column);

/* Renames the column identified by 'col_nr' to 'title' */
void list_column_rename(int col_nr, char *title);

/* Adds a new empty row to 'list' */
void list_row_add_empty(list_ *list);

/* Adds a new row with the values in 'values' to 'list' */
void list_row_add(list_ *list, char *values[]);

/* Deletes the rowS referenced by 'row_refs' from 'list'.
 * row_refs is a GList of pointers to GtkTreeRowReferences 
 */
void list_row_delete(list_ *list, GList *row_refs);

/* Creates and returns a new empty list */
list_ *list_create(void);

/* Sets the title of 'list' to 'title' */
void list_title_set(list_ *list, char *title);

/* Set the current filename of 'list' to 'filename'. 'filename' may be
 * a relative path+filename, absolute path+filename or filename.
 */
void list_filename_set(list_ *list, char *filename);

/* Clears and reinitializes 'list'. Afterwards, 'list' will be
 * like if it was generated with list_create()
 */
void list_clear(list_ *list);

/* Read 'filename' (containing fields separated by 'delimiter'; one row
 * per line) and puts this in 'list'. NOTE: 'list' should be empty (no 
 * columns, no rows) when importing csv!'
 */
int list_import_csv(list_ *list, char *filename, char delimiter);

/* Outputs the data in 'list' to 'file', separating each column value
 * with 'delimiter' and putting one row per line 
 */
int list_export_csv(list_ *list, char *filename, char delimiter);

/* Outputs the data in 'list' to 'file' in a PostScript format.
 * Ideally, the output should be fully suitable for printing. 
 * At the moment the output is rather shabby and unusable, and is
 * more a proof-of-concept than an actual feature.
 */
int list_export_ps(list_ *list, char *filename, int orientation);

/* Outputs the data in 'list' to 'file' in a HTML format. */
int list_export_html(list_ *list, char *filename);

/* Load a listpatron file (.lip) */
int list_load(list_ *list, char *filename);

/* Save a listpatron file (.lip) */
int list_save(list_ *list, char *filename);

/* Check if the list 'list' needs saving or if it doesn't have any
 * unsaved changes. Returns 1 if the list should be saved (the list 
 * will actually already be saved!! FIXME), -1 is the operation was
 * cancelled or 0 if the list doesn't need to be saved.
 * DEPRECATED: This function's inner workings and interfacing will
 * be changed in the future, please do not use (yet).
 */
int list_save_check(list_ *list);

/* Find an occurence of 'needle' in 'list', starting at the column and row
 * where the last occurence for this 'list' was found by previous calls to 
 * this function.
 *
 * When a new reference is found, it returns 1 and 'occ_row' and 'occ_col'
 * will point to the row and column of the occurence. If no further references 
 * are found, it returns 0 and 'occ_row' and 'occ_col' will be -1. 
 *
 * Finding 'needle' is influenced by 'options':
 * FIND_MATCHCASE (1) : 'needle' is searched for case sensitive
 * FIND_MATCHFULL (2) : Only columns which match 'needle' completely are
 *                      considered an occurence.
 */
int list_find(list_ *list, char *needle, int options, int *occ_row, int *occ_col);

#endif
