/****************************************************************************
 *
 * ListPatron
 *
 * A small GTK program for keeping lists of stuff.
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/
#ifndef LISTPATRON_H
#define LISTPATRON_H

#define _TEST_COLS 5
#define _TEST_ROWS 50

/* Personal GTK additions, sort of */
char *gtk_input_dialog(char *message, char *prefill);
void gtk_error_dialog(char *fmt, ...);
void gtk_statusbar_msg(char *fmt, ...);

/* Debugging functions */
void debug_msg(int dbg_type, char *file, int line, char *fmt, ...);

//void ui_sort_ok_cb(GtkListStore *ls_sort, GtkEntry *ent_name);
//gint ui_sort (void);
void ui_menu_sort_rules_cb(void);
void ui_menu_sort_edit_cb(void);

/* Menu callback functions */
void ui_menu_file_new_cb(void);
void ui_menu_file_import_csv_cb(void);
void ui_menu_file_export_csv_cb(void);
void ui_menu_file_export_ps_cb(void);
void ui_menu_file_export_html_cb(void);
void ui_menu_file_open_cb(void);
void ui_menu_file_save_cb(void);
void ui_menu_file_save_as_cb(void);
void ui_menu_file_quit_cb(void);
void ui_menu_edit_find_cb(void);
void ui_menu_column_add_cb(void);
void ui_menu_column_rename_cb(void);
void ui_menu_column_delete_cb(void);
void ui_menu_row_add_cb(void);
void ui_menu_row_delete_cb(void);
void ui_menu_debug_addtestdata_cb(void);
void ui_menu_debug_addtestrows_cb(void);
void ui_menu_debug_dumpsortrules_cb(void);
void ui_menu_help_usage_cb(void);
void ui_menu_help_about_cb(void);

/* Various Callback functions */
void ui_treeview_cursor_changed_cb(GtkTreeView *tv, gpointer user_data);
void ui_cell_edited_cb(GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data);
void ui_treeview_column_clicked_cb(GtkTreeViewColumn *col, gpointer *data);
void ui_treeview_columns_changed_cb(GtkTreeView *treeview, gpointer *data);

/* User interface creation functions */
void ui_create_menu_sortrules(GArray *rules);
GtkWidget *ui_create_menubar(GtkWidget *window);

#endif
