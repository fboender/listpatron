/****************************************************************************
 *
 * ListPatron - ui_import
 *
 * User Interface routines for importing files
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/
#ifndef UI_IMPORT_H
#define UI_IMPORT_H

/* Import CSV options */
#define IMPORT_CSV_HEADER         1 << 0 /* First row contains column header titles */
#define IMPORT_CSV_QUOTE_ENCLODED 1 << 1 /* Fields are enclosed in quotes (single or double) */

typedef struct import_ {
	char *filename;
	char delimiter;
} import_;

void ui_import_csv(void);

#endif
