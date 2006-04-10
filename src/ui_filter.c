/****************************************************************************
 *
 * ListPatron - ui_filter
 *
 * User Interface routines for the filtering rules editing
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib-object.h>
#include <assert.h>

#include "ui_filter.h"
#include "list.h"
#include "libgtkext.h"

extern list_ *list;

int ui_filter_new(void) {
	ui_filter_rule_edit(NULL);
	return(0);
}

int ui_filter_edit(char *rule_name) {
	ui_filter_rule_edit(rule_name);
	return(0);
}

int ui_filter_delete(char *rule_name) {
//	ui_filter_rule_delete(rule_name);
	return(0);
}

int ui_filter_ok_cb(ui_filter_ *ui_filter, GtkEntry *ent_name) {
	gchar *new_name = NULL;
	GArray *filter_predicates = NULL;
	int i;

	filter_predicates = g_array_new(FALSE, FALSE, sizeof(filter_predicate_ *));

	new_name = (gchar *)gtk_entry_get_text(ent_name);

	for (i = 0; i < ui_filter->predicates->len; i++) {
		ui_filter_predicate_ *ui_predicate;
		char *str_column = NULL, 
			 *str_predicate = NULL, 
			 *str_value = NULL, 
			 *str_operator = NULL;
		int col_nr;

		ui_predicate = g_array_index(ui_filter->predicates, ui_filter_predicate_ *, i);

		str_column = gtk_combo_box_get_active_string(GTK_COMBO_BOX(ui_predicate->cmb_column));
		str_predicate = gtk_combo_box_get_active_string(GTK_COMBO_BOX(ui_predicate->cmb_predicate));
		str_value = (char *)gtk_entry_get_text(GTK_ENTRY(ui_predicate->ent_value));
		str_operator = gtk_combo_box_get_active_string(GTK_COMBO_BOX(ui_predicate->cmb_operator));

		if (str_column != NULL && str_value != NULL && str_predicate != NULL) {
			filter_predicate_ *filter_predicate;

			filter_predicate = malloc(sizeof(filter_predicate_));

			filter_predicate->col_name = NULL;
			filter_predicate->col_nr = -1;
			filter_predicate->predicate = -1;
			filter_predicate->value = NULL;
			filter_predicate->bin_operator = -1;

			/* Store column name in filter predicate */
			filter_predicate->col_name = strdup(str_column);
			
			/* Find column nr and store in filter predicate */
			for (col_nr = 0; col_nr < list->columns->len; col_nr++) {
				char *col_name;

				col_name = g_array_index(list->columns, char *, col_nr);
				if (strcmp(col_name, str_column) == 0) {
					filter_predicate->col_nr = col_nr;
					break;
				}
			}

			/* Find out predicate int value and store in filter predicate */
			/* This is ugly, but otherwise I'll have to completely rewrite the dropdown code */
			if (strcasecmp(str_predicate, "is") == 0)                 { filter_predicate->predicate = IS;               } else
			if (strcasecmp(str_predicate, "is not") == 0)             { filter_predicate->predicate = IS_NOT;           } else
			if (strcasecmp(str_predicate, "contains") == 0)           { filter_predicate->predicate = CONTAINS;         } else
			if (strcasecmp(str_predicate, "does not contain") == 0)   { filter_predicate->predicate = DOES_NOT_CONTAIN; } else
			if (strcasecmp(str_predicate, "larger than") == 0)        { filter_predicate->predicate = LARGER_THAN;      } else
			if (strcasecmp(str_predicate, "smaller than") == 0)       { filter_predicate->predicate = SMALLER_THAN;     } else
			if (strcasecmp(str_predicate, "regular expression") == 0) { filter_predicate->predicate = REGULAR_EXP;      }

			/* Store value */
			filter_predicate->value = strdup(str_value);

			/* Find operator and store in filter predicate */
			if (str_operator != NULL) {
				/* Find out the operator */
				if (strcasecmp(str_operator, "and") == 0) { filter_predicate->bin_operator = AND; } else
				if (strcasecmp(str_operator, "or") == 0)  { filter_predicate->bin_operator = OR;  } 
			} else {
				filter_predicate->bin_operator = -1;
			}

			g_array_append_val(filter_predicates, filter_predicate);
		}

		g_free(str_column);
		g_free(str_predicate);
		g_free(str_operator);
	}
	
	list_filter_add(list, ui_filter->old_name, new_name, filter_predicates);

	return(0);
}

void ui_filter_rule_delete(char *rulename) {
	list_filter_remove(list, rulename);
}

ui_filter_predicate_ *ui_filter_create_predicate(
		filter_predicate_ *filter_predicate) {
	ui_filter_predicate_ *predicate;
	char *predicates[] = {
		"Is",
		"Is not",
		"Contains",
		"Does not contain",
		"Larger than",
		"Smaller than",
		"Regular expression",
		NULL
	};
	char *operators[] = {
		"And",
		"Or",
		NULL
	};
	int i;

	predicate = malloc(sizeof(struct ui_filter_predicate_));

	predicate->cmb_column  = gtk_combo_box_new_text();
	predicate->cmb_predicate = gtk_combo_box_new_text();
	predicate->ent_value = gtk_entry_new();
	predicate->cmb_operator = gtk_combo_box_new_text();
	
	for (i = 0; i < list->columns->len; i++) {
		char *col_name = NULL;

		col_name = g_array_index(list->columns, char *, i);

		gtk_combo_box_append_text(
				GTK_COMBO_BOX(predicate->cmb_column),
				col_name);
	}

	i = 0;
	while(predicates[i] != NULL) {
		gtk_combo_box_append_text(
				GTK_COMBO_BOX(predicate->cmb_predicate),
				predicates[i]);
		i++;
	}

	i = 0;
	while(operators[i] != NULL) {
		gtk_combo_box_append_text(
				GTK_COMBO_BOX(predicate->cmb_operator),
				operators[i]);
		i++;
	}

	/* If we're editing an existing predicate, set the correct values */
	if (filter_predicate != NULL) {
		gtk_combo_box_set_active(
				GTK_COMBO_BOX(predicate->cmb_column), 
				filter_predicate->col_nr);
		gtk_combo_box_set_active(
				GTK_COMBO_BOX(predicate->cmb_predicate), 
				filter_predicate->predicate);
		gtk_entry_set_text(
				GTK_ENTRY(predicate->ent_value), 
				filter_predicate->value);
		gtk_combo_box_set_active(
				GTK_COMBO_BOX(predicate->cmb_operator), 
				filter_predicate->bin_operator);
	}
	
	predicate->hbox_predicate = gtk_hbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(predicate->hbox_predicate), predicate->cmb_column, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(predicate->hbox_predicate), predicate->cmb_predicate, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(predicate->hbox_predicate), predicate->ent_value, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(predicate->hbox_predicate), predicate->cmb_operator, FALSE, FALSE, 3);

	return(predicate);
}

void ui_filter_operator_changed(GtkComboBox *cmb, struct ui_filter_ *ui_filter) {
	struct ui_filter_predicate_ *predicate;

	predicate = ui_filter_create_predicate(NULL);
	g_array_append_val(ui_filter->predicates, predicate);
	g_signal_connect(
			predicate->cmb_operator,
			"changed",
			GTK_SIGNAL_FUNC(ui_filter_operator_changed),
			ui_filter);
	gtk_box_pack_start(GTK_BOX(ui_filter->vbox_predicates), predicate->hbox_predicate, FALSE, FALSE, 3);
	gtk_widget_show_all(predicate->hbox_predicate);
}

gint ui_filter_rule_edit(char *name) {
	GtkWidget *dia_filter;
	GtkWidget *ent_name;
	gint response;
	GtkWidget *lbl_name;
	GtkWidget *hbox_name;
	struct ui_filter_ *ui_filter = NULL;
	struct ui_filter_predicate_ *predicate = NULL;
	int stop;
	
	if (list->nr_of_cols == 0) {
		gtk_error_dialog("The list has no columns to filter on");
		return(-1);
	}

	ui_filter = malloc(sizeof(struct ui_filter_));

	if (name != NULL) {
		ui_filter->old_name = strdup(name);
	}

	/* Create dialog */
	dia_filter = gtk_dialog_new_with_buttons(
			"Filter rule", /* FIXME: This is set wrong in ui_sort. Putting
							  this comment here so I won't forget to change
							  it there.*/
			NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			NULL);
	gtk_window_set_default_size(GTK_WINDOW(dia_filter), 350, 250);

	ui_filter->vbox_predicates = gtk_vbox_new(FALSE, 3);

	ui_filter->predicates  = g_array_new(
			FALSE, FALSE, sizeof(ui_filter_predicate_ *));


	/* Create filter name entry */
	hbox_name = gtk_hbox_new(FALSE, 3);
	lbl_name = gtk_label_new("Name");
	ent_name = gtk_entry_new();

	gtk_box_pack_start(GTK_BOX(hbox_name), lbl_name, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox_name), ent_name, TRUE, TRUE, 3);

	/* If we're editing a rule, load it. */
	if (name != NULL) {
		filter_ *filter = NULL;
		int i;

		filter = list_filter_getrule(list, name);

		assert(filter != NULL);

		gtk_entry_set_text(GTK_ENTRY(ent_name), filter->name);
		for (i = 0; i < filter->predicates->len; i++) {
			filter_predicate_ *filter_predicate = NULL;
			
			filter_predicate = g_array_index(filter->predicates, filter_predicate_ *, i);

			assert(filter_predicate != NULL);

			predicate = ui_filter_create_predicate(filter_predicate);
			g_array_append_val(ui_filter->predicates, predicate);

			gtk_box_pack_start(GTK_BOX(ui_filter->vbox_predicates), predicate->hbox_predicate, FALSE, FALSE, 3);
		}
	} else {
		/* Otherwise, show a single predicate */
		predicate = ui_filter_create_predicate(NULL);
		g_array_append_val(ui_filter->predicates, predicate);
		gtk_box_pack_start(GTK_BOX(ui_filter->vbox_predicates), predicate->hbox_predicate, FALSE, FALSE, 3);
	}

	/* Choosing something from the last operator adds another predicate;
	 * Connect signal to the last operator dropdown */
	g_signal_connect(
			predicate->cmb_operator,
			"changed",
			GTK_SIGNAL_FUNC(ui_filter_operator_changed),
			ui_filter);

	/* Put everything in the dialog's vbox */
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_filter)->vbox), GTK_WIDGET(hbox_name), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_filter)->vbox), GTK_WIDGET(ui_filter->vbox_predicates), FALSE, FALSE, 3);

	gtk_widget_show_all(dia_filter);

	stop = 0;
	while (!stop) {
		response = gtk_dialog_run(GTK_DIALOG(dia_filter));
		switch (response) {
			case GTK_RESPONSE_ACCEPT: 
				if (ui_filter_ok_cb(ui_filter, GTK_ENTRY(ent_name)) == 0) {
					stop = 1;
				}
				break;
			case GTK_RESPONSE_REJECT:
				stop = 1;
			default:
				break;
		}
	}

//	/* FIXME: Free used memory. This is currently not done, but should be done
//	 * for a.o. rules list, iters?, etc */
	gtk_widget_destroy(dia_filter);

	return (response);
}

