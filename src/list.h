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

/* Import CSV options */
#define IMPORT_CSV_HEADER         1 << 0 /* First row contains column header titles */
#define IMPORT_CSV_QUOTE_ENCLODED 1 << 1 /* Fields are enclosed in quotes (single or double) */

/* Export Printable (PS) options */
#define ORIENT_PORTRAIT 0
#define ORIENT_LANDSCAPE 1

/* List find options */
#define FIND_MATCHCASE 1
#define FIND_MATCHFULL 2

typedef struct list_ {
	char *filename;

	/* Info */
	char *version;
	char *title;
	char *author;
	char *description;
	char *keywords;

	GtkListStore *liststore; /* Row information */
	GArray *columns; /* Column titles */
	
	int nr_of_cols;
	int nr_of_rows;
	int modified;

	int last_occ_col;
	int last_occ_row;
} list_;

typedef struct import_ {    /*  Move to listpatron.c */
	char *filename;
	char delimiter;
} import_;

typedef struct export_ { /*  Move to listpatron.c */
	char *filename;

	int orientation;

	char delimiter;
} export_;

typedef struct find_ {/*  Move to listpatron.c */
	GtkWidget *ent_needle;
	int matchcase;
	int matchfull;
} find_;

void list_column_add(list_ *list, char *title);
void list_column_delete(list_ *list, GtkTreeViewColumn *column);
void list_row_add_empty(list_ *list);
void list_row_add(list_ *list, char *values[]);
void list_row_delete(list_ *list, GList *row_refs);
list_ *list_create(void);
void list_title_set(char *title);
void list_clear(void);
int list_import_csv(list_ *list, char *filename, char delimiter);
int list_export_csv(list_ *list, char *filename, char delimiter);
void ui_file_export_ps_portrait_cb(GtkWidget *radio, export_ *export);
void ui_file_export_ps_landscape_cb(GtkWidget *radio, export_ *export);
int list_export_ps(list_ *list, char *filename, int orientation);
int list_export_html(list_ *list, char *filename);
int list_load(list_ *list, char *filename);
int list_save(list_ *list, char *filename);
int list_save_check(list_ *list);
int list_find(list_ *list, char *needle, int options, int *occ_row, int *occ_col);

#endif
