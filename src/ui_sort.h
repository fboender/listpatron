/****************************************************************************
 *
 * ListPatron - ui_sort
 *
 * User Interface routines for the sorting rules editing
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/
#ifndef UI_SORT_H
#define UI_SORT_H

typedef struct ui_sort_ {
	char *old_name;
	GtkListStore *ls_sort;
	GtkWidget *frm_sortorder;
	int sort_order; /* FIXME: Unused? */
	GtkWidget *hbox_listctrl;
	GtkWidget *vbox_listitemmod;
	GtkTreeSelection *treeselection;
	GtkWidget *radio_sortasc, *radio_sortdesc;
} ui_sort_;

int ui_sort_new(void);
int ui_sort_edit(char *rule_name);
int ui_sort_delete(char *rule_name);
int ui_sort_ok_cb(ui_sort_ *ui_sort, GtkListStore *ls_sort, GtkEntry *ent_name);
void ui_sort_selection_changed_cb(GtkTreeSelection *treeselection, struct ui_sort_ *sort);
void ui_sort_moveup_clicked_cb(GtkButton *button, struct ui_sort_ *sort);
void ui_sort_movedown_clicked_cb(GtkButton *button, struct ui_sort_ *sort);
void ui_sort_sortasc_toggled_cb(GtkWidget *radio, ui_sort_ *sort);
void ui_sort_sortdesc_toggled_cb(GtkWidget *radio, ui_sort_ *sort);
void ui_sort_rule_delete(char *rulename);
void ui_sort_rule_edit_load(ui_sort_ *ui_sort, char *rulename, GtkListStore *ls_sort, GtkEntry *ent_name);
gint ui_sort_rule_edit(char *name);
gint ui_sort_rules(void);

#endif /* UI_SORT_H */
