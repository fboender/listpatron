/****************************************************************************
 *
 * ListPatron - libgtkext
 *
 * GTK extensions
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <glib-object.h>

/* FIXME: These next declarations should not be neccessary. Pass them to the
 * function instead 
 */
extern GtkWidget *win_main;  /* listpatron.c */
extern guint sb_context_id;
extern GtkWidget *sb_status;

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
		if (n >= err_len) { /* Throw some more mem at the buf */
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
		if (n >= msg_len) { /* Throw some more mem at the buf */
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


