/****************************************************************************
 *
 * ListPatron - ui_rulelist
 *
 * User Interface routines for displaying a list of rules and the possible
 * actions on those rules.
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

#include "ui_rulelist.h"
#include "list.h"
#include "libgtkext.h"

extern list_ *list;
extern GtkTreeView *treeview;

void ui_rulelist_populate(ui_rulelist_ *ui_rulelist) {
	int i;
	GtkTreeIter iter;

	/* Clear list */
	if (ui_rulelist->ls_rulelist != NULL) {
		gchar *row_data;
		
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ui_rulelist->ls_rulelist), &iter)) {
			do {
				gtk_tree_model_get(GTK_TREE_MODEL(ui_rulelist->ls_rulelist), &iter, 0, &row_data, -1);
				free(row_data);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(ui_rulelist->ls_rulelist), &iter));

			gtk_list_store_clear(ui_rulelist->ls_rulelist);
		}
	}
	
	/* (Re)populate list */
	for (i = 0; i < ui_rulelist->rules->len; i++) {
		rule_ *rule;

		rule = g_array_index(ui_rulelist->rules, rule_ *, i);

		gtk_list_store_append(ui_rulelist->ls_rulelist, &iter);
		gtk_list_store_set(ui_rulelist->ls_rulelist, &iter, 0, (char *)rule->name, -1);
	}
}

void ui_rulelist_new(GtkButton *button, struct ui_rulelist_ *ui_rulelist) {
	ui_rulelist->new_func();
	ui_rulelist_populate(ui_rulelist);
}
void ui_rulelist_edit(GtkButton *button, struct ui_rulelist_ *ui_rulelist) {
	ui_rulelist->edit_func(ui_rulelist->rule_name);
	ui_rulelist_populate(ui_rulelist);
}
void ui_rulelist_del(GtkButton *button, struct ui_rulelist_ *ui_rulelist) {
	ui_rulelist->del_func(ui_rulelist->rule_name);
	ui_rulelist_populate(ui_rulelist);
}

/* FIXME: itertjes enzo opruimen? */
void ui_rulelist_selection_changed_cb(
		GtkTreeSelection *treeselection, 
		ui_rulelist_ *ui_rulelist) {
	int selection_exists;
	GtkTreeIter iter;

	assert(ui_rulelist != NULL);
	assert(ui_rulelist->treeselection != NULL);

	selection_exists = gtk_tree_selection_get_selected(ui_rulelist->treeselection, NULL, &iter);
	
	if (selection_exists) {
		char *rule_name = NULL;
		
		/* Get the selected rule's name for possible editing/deletion */
		gtk_tree_model_get(GTK_TREE_MODEL(ui_rulelist->ls_rulelist), &iter, 0, &rule_name, -1);
		
		if (ui_rulelist->rule_name == NULL) {
			free(ui_rulelist->rule_name);
		}
		ui_rulelist->rule_name = strdup(rule_name);

		/* Enable the buttons for possible editing/deletion */
		gtk_widget_set_sensitive(ui_rulelist->btn_edit, TRUE);
		gtk_widget_set_sensitive(ui_rulelist->btn_del, TRUE);
	} else {
		/* No rule selected -> no editing/deleting */
		gtk_widget_set_sensitive(ui_rulelist->btn_edit, FALSE);
		gtk_widget_set_sensitive(ui_rulelist->btn_del, FALSE);
	}
}

gint ui_rulelist(
		char *title, 
		char *item_desc, 
		GArray *rules,
		int (*new_func)(void),
		int (*edit_func)(char *rule_name),
		int (*del_func)(char *rule_name)) {
	GtkWidget *dia_rulelist;
	ui_rulelist_ *ui_rulelist;

	GtkTreeView *tv_rulelist;
	GtkCellRenderer *renderer;
	GtkWidget *scrl_rulelist;

	GtkWidget *hbox_rulectrl;

	gint response;
	
	/* Init some stuff */
	ui_rulelist = malloc(sizeof(ui_rulelist_));
	ui_rulelist->rules = rules;
	ui_rulelist->rule_name = NULL;
	ui_rulelist->new_func = new_func;
	ui_rulelist->edit_func = edit_func;
	ui_rulelist->del_func = del_func;
	
	/* Create dialog */
	dia_rulelist = gtk_dialog_new_with_buttons(
			title,
			NULL,
			GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			NULL);

	/* Create liststore and put sort rules in list */
	ui_rulelist->ls_rulelist = gtk_list_store_new(
			1, 
			G_TYPE_STRING); /* Rule name (shown) */
	
	ui_rulelist_populate(ui_rulelist);

	/* Create treeview */
	tv_rulelist = GTK_TREE_VIEW(gtk_tree_view_new());
	gtk_tree_view_set_rules_hint(tv_rulelist, 1);
	gtk_tree_view_set_reorderable (tv_rulelist, TRUE);
	
	renderer = gtk_cell_renderer_text_new();
	gtk_tree_view_insert_column_with_attributes(
			tv_rulelist,
			0,
			item_desc,
			renderer,
			"text", 0,
			NULL);
	
	gtk_tree_view_set_model(tv_rulelist, GTK_TREE_MODEL(ui_rulelist->ls_rulelist));

	/* Treeview's selection */
	ui_rulelist->treeselection = gtk_tree_view_get_selection(tv_rulelist);
	gtk_tree_selection_set_mode(ui_rulelist->treeselection, GTK_SELECTION_SINGLE);
	g_signal_connect(
			ui_rulelist->treeselection,
			"changed",
			GTK_SIGNAL_FUNC(ui_rulelist_selection_changed_cb),
			ui_rulelist);
	
	scrl_rulelist = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrl_rulelist), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(scrl_rulelist), GTK_WIDGET(tv_rulelist));

	/* Create the list control buttons */
	ui_rulelist->btn_new = gtk_button_new_from_stock(GTK_STOCK_NEW);
	ui_rulelist->btn_edit = gtk_button_new_from_stock(GTK_STOCK_PROPERTIES);
	ui_rulelist->btn_del = gtk_button_new_from_stock(GTK_STOCK_DELETE);
	
	g_signal_connect(ui_rulelist->btn_new, "clicked", GTK_SIGNAL_FUNC(ui_rulelist_new), ui_rulelist);
	g_signal_connect(ui_rulelist->btn_edit, "clicked", GTK_SIGNAL_FUNC(ui_rulelist_edit), ui_rulelist);
	g_signal_connect(ui_rulelist->btn_del, "clicked", GTK_SIGNAL_FUNC(ui_rulelist_del), ui_rulelist);

	gtk_widget_set_sensitive(ui_rulelist->btn_edit, FALSE);
	gtk_widget_set_sensitive(ui_rulelist->btn_del, FALSE);

	hbox_rulectrl = gtk_hbox_new(TRUE, 0);
	gtk_box_pack_start_defaults(GTK_BOX(hbox_rulectrl), ui_rulelist->btn_new);
	gtk_box_pack_start_defaults(GTK_BOX(hbox_rulectrl), ui_rulelist->btn_edit);
	gtk_box_pack_start_defaults(GTK_BOX(hbox_rulectrl), ui_rulelist->btn_del);
	
	/* Put everything in the dialog's vbox */
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_rulelist)->vbox), GTK_WIDGET(scrl_rulelist), TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dia_rulelist)->vbox), GTK_WIDGET(hbox_rulectrl), FALSE, FALSE, 3);

	gtk_widget_show_all(dia_rulelist);

	response = gtk_dialog_run(GTK_DIALOG(dia_rulelist));

	/*********************************************************************
	 * TIJDELIJK: Sorting rule die geselecteerd is als sorting rule 
	 * instellen
	 *********************************************************************/
	{
		int selection_exists;
		GtkTreeIter iter;

		selection_exists = gtk_tree_selection_get_selected(ui_rulelist->treeselection, NULL, &iter);
	
		if (selection_exists) {
			char *rule_name = NULL;
			sort_ *sort;
			GList *columns = NULL;
			GList *col_iter = NULL;
			
			/* Get the selected rule's name for possible editing/deletion */
			gtk_tree_model_get(GTK_TREE_MODEL(ui_rulelist->ls_rulelist), &iter, 0, &rule_name, -1);
			
			/* FIXME: Remove external reference to list of done */
			/* FIXME list_sort_getrule returns sort_ *, but sort_active is
			 * GArray.. perhaps change sort_active to be sort_ * and change
			 * list_sort_func() to use sort_ * instead of cols */
			sort = list_sort_getrule(rule_name);
			list->sort_active = sort->columns;

			gtk_tree_sortable_set_default_sort_func(
					GTK_TREE_SORTABLE(list->liststore), 
					list_sort_func,
					NULL,
					NULL);

			gtk_tree_sortable_set_sort_column_id(
					GTK_TREE_SORTABLE(list->liststore),
					GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
					GTK_SORT_ASCENDING /* FIXME: Does it matter? */);

			/* Set column sort direction arrow indicators */
			columns = gtk_tree_view_get_columns(treeview);
			col_iter = columns;
			while (col_iter != NULL) {
				int col_nr;
				int col_sort_dir;
				sort_col_ *sort_col;
				int i;
				
				col_nr = GPOINTER_TO_UINT(
						g_object_get_data(G_OBJECT(col_iter->data), "col_nr"));
				
				/* Find the sort_order for this column */
				for (i = 0; i < sort->columns->len; i++) {
					sort_col = g_array_index(
							sort->columns,
							sort_col_ *,
							i);
					if (sort_col->col_nr == col_nr) {
						break;
					}
				}
				
				col_sort_dir = sort_col->sort_order;

//				sort_col = g_array_index(
//						sort->columns,
//						sort_col_ *,
//						col_nr);
//				col_sort_dir = sort_col->sort_order;
				
				gtk_tree_view_column_set_sort_indicator(
						GTK_TREE_VIEW_COLUMN(col_iter->data),
						TRUE);
				gtk_tree_view_column_set_sort_order(
						GTK_TREE_VIEW_COLUMN(col_iter->data),
						col_sort_dir);
				
				
				col_iter = col_iter->next;
			}

		}
	}

	/* FIXME: Memory onzin opruimen */
	free(ui_rulelist);

	gtk_widget_destroy(dia_rulelist);

	return (response);
}


