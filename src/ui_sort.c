/****************************************************************************
 *
 * ListPatron - ui_menu
 *
 * User Interface routines for the sorting rules editing
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

#include "ui_sort.h"
#include "list.h"
#include "libgtkext.h"

extern list_ *list;

int ui_sort_new(void) {
	ui_sort_rule_edit(NULL);
	return(0);
}

int ui_sort_edit(char *rule_name) {
	ui_sort_rule_edit(rule_name);
	return(0);
}

int ui_sort_delete(char *rule_name) {
	ui_sort_rule_delete(rule_name);
	return(0);
}

int ui_sort_ok_cb(ui_sort_ *ui_sort, GtkListStore *ls_sort, GtkEntry *ent_name) {
	GtkTreeIter iter;
	GArray *sort_cols;
	gchar *new_name = NULL;
	int i;

	new_name = (gchar *)gtk_entry_get_text(ent_name);
	
	/* Determine if no other rules with the same name exist */
	if (strcmp(ui_sort->old_name, new_name) != 0) { /* Name has changed */
		for (i = 0; i < list->sorts->len; i++) {
			sort_ *sort;

			sort = g_array_index(list->sorts, sort_ *, i);
			if (strcmp(sort->name, new_name) == 0) {
				int response;
				/* Duplicate found */
				response = gtk_yesno_dialog("Overwrite?", "A rule with this name already exists. Do you wish to overwrite the rule?");
				if (response == GTK_RESPONSE_YES) {
					list_sort_remove(list, new_name);
				} else {
					return(-1);
				}
			}
		}
	}

	sort_cols = g_array_new(FALSE, FALSE, sizeof(sort_col_ *));

	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ls_sort), &iter);
	do {
		sort_col_ *sort_col;
		
		sort_col = malloc(sizeof(sort_col_)); /* Freed by list_sort_remove() */
		
		gtk_tree_model_get(GTK_TREE_MODEL(ls_sort), &iter, 0, &(sort_col->col_name), -1);
		gtk_tree_model_get(GTK_TREE_MODEL(ls_sort), &iter, 1, &(sort_col->col_nr), -1);
		gtk_tree_model_get(GTK_TREE_MODEL(ls_sort), &iter, 2, &(sort_col->sort_order), -1);

		g_array_append_val(sort_cols, sort_col);
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(ls_sort), &iter));

	list_sort_add(list, ui_sort->old_name, new_name, sort_cols);
	
	return(0);
}

void ui_sort_selection_changed_cb(GtkTreeSelection *treeselection, struct ui_sort_ *sort) {
	GtkTreeIter iter;
	gint sort_order;

	if (!gtk_tree_selection_get_selected(sort->treeselection, NULL, &iter)) {
		gtk_widget_set_sensitive(sort->vbox_listitemmod, FALSE);
		return;
	}
	
	gtk_tree_model_get(GTK_TREE_MODEL(sort->ls_sort), &iter, 2, &sort_order, -1);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sort->radio_sortasc), sort_order == GTK_SORT_ASCENDING);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(sort->radio_sortdesc), sort_order == GTK_SORT_DESCENDING);
	
	gtk_widget_set_sensitive(sort->vbox_listitemmod, TRUE);
}

void ui_sort_moveup_clicked_cb(GtkButton *button, struct ui_sort_ *sort) {
	GtkTreeIter iter_a, iter_b;
	GtkTreePath *path_a, *path_b;
	
	if (!gtk_tree_selection_get_selected(sort->treeselection, NULL, &iter_a)) {
		fprintf(stderr, "Strange. No selection?\n");
		return;
	}

	path_a = gtk_tree_model_get_path(GTK_TREE_MODEL(sort->ls_sort), &iter_a);
	path_b = path_a;

	if (!gtk_tree_path_prev(path_b)) {
		path_b = path_a;
	}

	gtk_tree_model_get_iter(GTK_TREE_MODEL(sort->ls_sort), &iter_b, path_b);
	
	gtk_list_store_swap(sort->ls_sort, &iter_a, &iter_b);

}

void ui_sort_movedown_clicked_cb(GtkButton *button, struct ui_sort_ *sort) {
	GtkTreeIter iter_a, iter_b;
	GtkTreePath *path_a, *path_b;
	
	/* Doing this the hard way because you cant copy an iter, so we can't use iter_next() */

	if (!gtk_tree_selection_get_selected(sort->treeselection, NULL, &iter_a)) {
		fprintf(stderr, "Strange. No selection?\n");
		return;
	}

	path_a = gtk_tree_model_get_path(GTK_TREE_MODEL(sort->ls_sort), &iter_a);
	path_b = path_a;

	gtk_tree_path_next(path_b); /* Why doesn't this return true/false? */
	if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sort->ls_sort), &iter_b, path_b)) {
		gtk_list_store_swap(sort->ls_sort, &iter_a, &iter_b);
	}
}

void ui_sort_sortasc_toggled_cb(GtkWidget *radio, ui_sort_ *sort) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		GtkTreeIter iter;
		
		if (!gtk_tree_selection_get_selected(sort->treeselection, NULL, &iter)) {
			fprintf(stderr, "Strange. No selection?\n");
			return;
		}
		
		gtk_list_store_set(sort->ls_sort, &iter, 2, (int)GTK_SORT_ASCENDING, -1);
	}
}

void ui_sort_sortdesc_toggled_cb(GtkWidget *radio, ui_sort_ *sort) {
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio)) == 1) {
		GtkTreeIter iter;
		
		if (!gtk_tree_selection_get_selected(sort->treeselection, NULL, &iter)) {
			fprintf(stderr, "Strange. No selection?\n");
			return;
		}
		
		gtk_list_store_set(sort->ls_sort, &iter, 2, (int)GTK_SORT_DESCENDING, -1);
	}
}

void ui_sort_rule_delete(char *rulename) {
	list_sort_remove(list, rulename);
}

void ui_sort_rule_edit_load(ui_sort_ *ui_sort, char *rulename, GtkListStore *ls_sort, GtkEntry *ent_name) {
	sort_ *sort_rule;
	int i;
	GtkTreeIter iter;

	if (rulename == NULL) { /* Create new rule */
		gtk_entry_set_text(ent_name, "Unnamed");
		for (i = 0; i < list->columns->len; i++) {
			char *col_name = NULL;

			col_name = g_array_index(list->columns, char *, i);

			gtk_list_store_append(ls_sort, &iter);
			gtk_list_store_set(ls_sort, &iter, 0, (char *)col_name, -1);
			gtk_list_store_set(ls_sort, &iter, 1, (int)i, -1);
			gtk_list_store_set(ls_sort, &iter, 2, (int)GTK_SORT_ASCENDING, -1);
		}
	} else { /* Edit existing rule */
		sort_rule = list_sort_getrule(list, rulename);
		
		assert(sort_rule != NULL);
		assert(sort_rule->name != NULL);

		gtk_entry_set_text(ent_name, sort_rule->name);
		for (i = 0; i < sort_rule->columns->len; i++) {
			sort_col_ *sort_col;

			sort_col = g_array_index(sort_rule->columns, sort_col_ *, i);

			gtk_list_store_append(ls_sort, &iter);
			gtk_list_store_set(ls_sort, &iter, 0, (char *)sort_col->col_name, -1);
			gtk_list_store_set(ls_sort, &iter, 1, (int)sort_col->col_nr, -1);
			gtk_list_store_set(ls_sort, &iter, 2, (int)sort_col->sort_order, -1);
		}
	}
}

gint ui_sort_rule_edit(char *name) {
	GtkCellRenderer *renderer;
	GtkWidget *ent_name;
	GtkWidget *dia_sort;
	GtkTreeView *tv_sort;
	gint response;
	GtkWidget *lbl_name;
	GtkWidget *hbox_name;
	GtkWidget *btn_moveup, *btn_movedown;
	GtkWidget *scrl_list;
	GtkWidget *vbox_sortorder;
	struct ui_sort_ *sort;
	int stop;
	
	if (list->nr_of_cols == 0) {
		gtk_error_dialog("The list has no columns to sort on");
		return(-1);
	}

	sort = malloc(sizeof(struct ui_sort_));
	sort->vbox_listitemmod = NULL;
	sort->old_name = NULL;

	/* Create dialog */
	dia_sort = gtk_dialog_new_with_buttons(
			"Sort list",
			NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			NULL);
	gtk_window_set_default_size(GTK_WINDOW(dia_sort), 350, 250);

	/* Create sort name entry */
	hbox_name = gtk_hbox_new(FALSE, 3);
	lbl_name = gtk_label_new("Name");
	ent_name = gtk_entry_new();

	gtk_box_pack_start(GTK_BOX(hbox_name), lbl_name, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(hbox_name), ent_name, TRUE, TRUE, 3);

	/* Create liststore and put column names in list */
	sort->ls_sort = gtk_list_store_new(
			3, 
			G_TYPE_STRING, /* Column name (shown) */
			G_TYPE_INT,    /* Column nr */
			G_TYPE_INT);   /* Sort direction */

	/* Create treeview */
	tv_sort = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_tree_view_set_rules_hint(tv_sort, 1);
	gtk_tree_view_set_reorderable (tv_sort, TRUE);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(
			tv_sort,
			0,
			"Column",
			renderer,
			"text", 0,
			NULL);
	
	gtk_tree_view_set_model(tv_sort, GTK_TREE_MODEL(sort->ls_sort));

	/* Treeview's selection */
	sort->treeselection = gtk_tree_view_get_selection(tv_sort);
	gtk_tree_selection_set_mode(sort->treeselection, GTK_SELECTION_SINGLE);
	g_signal_connect(
			sort->treeselection,
			"changed",
			GTK_SIGNAL_FUNC(ui_sort_selection_changed_cb),
			sort);
	
	scrl_list = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrl_list), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrl_list), GTK_WIDGET(tv_sort));

	/* List item controls */
	btn_moveup = gtk_button_new_from_stock(GTK_STOCK_GO_UP);
	btn_movedown = gtk_button_new_from_stock(GTK_STOCK_GO_DOWN);

	g_signal_connect(btn_moveup, "clicked", GTK_SIGNAL_FUNC(ui_sort_moveup_clicked_cb), sort);
	g_signal_connect(btn_movedown, "clicked", GTK_SIGNAL_FUNC(ui_sort_movedown_clicked_cb), sort);

	sort->radio_sortasc = gtk_radio_button_new_with_mnemonic(
			NULL, 
			"Sort _Ascending");
	sort->radio_sortdesc = gtk_radio_button_new_with_mnemonic_from_widget(
			GTK_RADIO_BUTTON(sort->radio_sortasc),
			"Sort D_escending");

	g_signal_connect(
			sort->radio_sortasc, 
			"toggled",
			GTK_SIGNAL_FUNC(ui_sort_sortasc_toggled_cb),
			sort);
	g_signal_connect(
			GTK_OBJECT(sort->radio_sortdesc), 
			"toggled",
			GTK_SIGNAL_FUNC(ui_sort_sortdesc_toggled_cb),
			sort);

	vbox_sortorder = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_sortorder), sort->radio_sortasc, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_sortorder), sort->radio_sortdesc, FALSE, FALSE, 0);

	sort->frm_sortorder = gtk_frame_new("Sorting order");
	gtk_container_set_border_width(GTK_CONTAINER(vbox_sortorder), 5);
	gtk_container_add(GTK_CONTAINER(sort->frm_sortorder), vbox_sortorder);

	/* Put all list item controls in a vbox */
	sort->vbox_listitemmod = gtk_vbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(sort->vbox_listitemmod), btn_moveup, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(sort->vbox_listitemmod), btn_movedown, FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(sort->vbox_listitemmod), sort->frm_sortorder, FALSE, FALSE, 3);

	/* List and list control */
	sort->hbox_listctrl = gtk_hbox_new(FALSE, 3);
	gtk_box_pack_start(GTK_BOX(sort->hbox_listctrl), scrl_list, TRUE, TRUE, 3);
	gtk_box_pack_start(GTK_BOX(sort->hbox_listctrl), sort->vbox_listitemmod, FALSE, FALSE, 3);

	/* Put everything in the dialog's vbox */
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_sort)->vbox), GTK_WIDGET(hbox_name), FALSE, FALSE, 3);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_sort)->vbox), GTK_WIDGET(sort->hbox_listctrl), TRUE, TRUE, 3);

	/* By default, the list item controls are disabled */
	gtk_widget_set_sensitive(sort->vbox_listitemmod, FALSE);

	/* Edit sorting rule */
	ui_sort_rule_edit_load(sort, name, sort->ls_sort, GTK_ENTRY(ent_name));
	if (name != NULL) {
		sort->old_name = strdup(name);
	} else {
		sort->old_name = strdup("Unnamed");
	}

	gtk_widget_show_all(dia_sort);

	stop = 0;
	while (!stop) {
		response = gtk_dialog_run(GTK_DIALOG(dia_sort));
		switch (response) {
			case GTK_RESPONSE_ACCEPT: 
				if (ui_sort_ok_cb(sort, sort->ls_sort, GTK_ENTRY(ent_name)) == 0) {
					stop = 1;
				}
				break;
			case GTK_RESPONSE_REJECT:
				stop = 1;
			default:
				break;
		}
	}

	/* FIXME: Free used memory. This is currently not done, but should be done
	 * for a.o. rules list, iters?, etc */

	gtk_widget_destroy(dia_sort);

	return (response);
}

