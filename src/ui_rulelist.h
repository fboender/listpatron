/****************************************************************************
 *
 * ListPatron - ui_menu
 *
 * User Interface routines for a generic rules editing dialog. The dialog
 * shows a list of rules (sorts, filters, reports) and allows the user to
 * pick an action to perform on the rule (new, edit, delete).
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/
#ifndef UI_RULELIST_H
#define UI_RULELIST_H

typedef struct ui_rulelist_ {
	GArray *rules;
	GtkTreeSelection *treeselection;
	GtkListStore *ls_rulelist;
	GtkWidget *btn_new, *btn_edit, *btn_del;
	char *rule_name;
	int (*new_func)(void);
	int (*edit_func)(char *rule_name);
	int (*del_func)(char *rule_name);
} ui_rulelist_;

gint ui_rulelist(
		char *title, 
		char *item_desc, 
		GArray *rules,
		int (*new_func)(void),
		int (*edit_func)(char *rule_name),
		int (*del_func)(char *rule_name));

#endif
