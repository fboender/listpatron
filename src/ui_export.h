/****************************************************************************
 *
 * ListPatron - ui_export
 *
 * User Interface routines for exporting files
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/
#ifndef UI_EXPORT_H
#define UI_EXPORT_H

typedef struct export_ { /*  Move to listpatron.c */
	char *filename;
	int orientation;
	char delimiter;
} export_;

//void ui_export_html_cb(void);
//void ui_export_ps_cb(void);

//void ui_export_ps_portrait_cb(GtkWidget *radio, export_ *export) {
//void ui_export_ps_landscape_cb(GtkWidget *radio, export_ *export) {
//void ui_export_delimiter_comma_cb(GtkWidget *radio, export_ *export) {
//void ui_export_delimiter_tab_cb(GtkWidget *radio, export_ *export) {
void ui_export_csv(void);
void ui_export_ps(void);
void ui_export_html(void);

#endif
