/****************************************************************************
 *
 * ListPatron - libgtkext
 *
 * GTK extentions
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#ifndef LIBGTKEXT_H
#define LIBGTKEXT_H

void gtk_input_dialog_entry_activate_cb(GtkWidget *entry, GtkDialog *dia_input);
char *gtk_input_dialog(char *message, char *prefill); /* Result must be freed */
void gtk_statusbar_msg(char *fmt, ...);
void gtk_error_dialog(char *fmt, ...);

#endif

