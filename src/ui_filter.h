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
#ifndef UI_FILTER_H
#define UI_FILTER_H

typedef struct ui_filter_predicate_ {
	GtkWidget *hbox_predicate;
	GtkWidget *cmb_column,
			  *cmb_predicate,
			  *cmb_operator;
	GtkWidget *ent_value;
} ui_filter_predicate_;

typedef struct ui_filter_ {
	char *old_name;
	GtkWidget *vbox_predicates;
	GArray *predicates; /* ui_filter_predicate_ * */
} ui_filter_;

int ui_filter_new(void);
int ui_filter_edit(char *rule_name);
int ui_filter_delete(char *rule_name);
gint ui_filter_rule_edit(char *name);
gint ui_filter_rules(void);

#endif /* UI_FILTER_H */
