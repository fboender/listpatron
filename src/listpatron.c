#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <gtk/gtk.h>
#include <glib-object.h>

/****************************************************************************/

typedef struct list_ {
	GtkTreeView *treeview;
	GtkListStore *liststore;
} list_;

/****************************************************************************
 * Prototyping
 ****************************************************************************/
/* List handling functions */
void list_columns_set (list_ *list, int nr_of_cols, char *titles[]);
void list_column_add (list_ *list, char *title);
void list_column_delete (list_ *list);
void list_row_add (list_ *list, int nr_of_cols, char *values[]);
list_ *list_create (void);
/* Callback functions */
void ui_menu_file_open_cb (void);
void ui_menu_column_add_cb (void);
void ui_menu_column_rename_cb (void);
void ui_menu_column_delete_cb (void);
void ui_cell_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data);
/* User interface creation functions */
GtkWidget *ui_create_menubar (GtkWidget *window);

/****************************************************************************
 * Data initializer
 ****************************************************************************/
static GtkItemFactoryEntry ui_menu_items[] = {
	{ "/_File"         , NULL , NULL          , 0 , "<Branch>"                       } ,
	{ "/File/_New"     , NULL , list_column_add          , 0 , "<StockItem>"  , GTK_STOCK_NEW   } ,
	{ "/File/_Open"    , NULL , ui_menu_file_open_cb          , 0 , "<StockItem>"  , GTK_STOCK_OPEN  } ,
	{ "/File/_Save"    , NULL , NULL          , 0 , "<StockItem>"  , GTK_STOCK_SAVE  } ,
	{ "/File/Save _As" , NULL , NULL          , 0 , "<Item>"                         } ,
	{ "/File/_Export"  , NULL , NULL          , 0 , "<Item>"                         } ,
	{ "/File/sep1"     , NULL , NULL          , 0 , "<Separator>"                    } ,
	{ "/File/_Quit"    , NULL , gtk_main_quit , 0 , "<StockItem>"  , GTK_STOCK_QUIT  } ,
	{ "/_Edit"         , NULL , NULL          , 0 , "<Branch>"                       } ,
	{ "/Edit/Cu_t"     , NULL , NULL          , 0 , "<StockItem>"  , GTK_STOCK_CUT   } ,
	{ "/Edit/_Copy"    , NULL , NULL          , 0 , "<StockItem>"  , GTK_STOCK_COPY  } ,
	{ "/Edit/_Paste"   , NULL , NULL          , 0 , "<StockItem>"  , GTK_STOCK_PASTE } ,
	{ "/Edit/sep1"     , NULL , NULL          , 0 , "<Separator>"                    } ,
	{ "/_Column"         , NULL , NULL          , 0 , "<Branch>"                       } ,
	{ "/Column/_Add"     , NULL , ui_menu_column_add_cb  , 0 , "<StockItem>", GTK_STOCK_ADD   } ,
	{ "/Column/_Delete"    , NULL , ui_menu_column_delete_cb  , 0 , "<StockItem>", GTK_STOCK_DELETE  } ,
	{ "/Column/_Rename"    , NULL , ui_menu_column_rename_cb  , 0 , "<Item>"  } ,
	{ "/_Help"         , NULL , NULL          , 0 , "<LastBranch>"                   } ,
	{ "/_Help/About"   , NULL , NULL          , 0 , "<Item>"                         } ,
};

static gint ui_nmenu_items = sizeof (ui_menu_items) / sizeof (ui_menu_items[0]);

list_ *list = NULL;

/****************************************************************************
 * gtk_input_dialog
 ****************************************************************************/
char *gtk_input_dialog (char *message, char *prefill) {
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
	
	ent_input = gtk_entry_new ();
	gtk_entry_set_text (GTK_ENTRY(ent_input), prefill);

	gtk_box_pack_start (GTK_BOX(GTK_DIALOG(dia_input)->vbox), GTK_WIDGET(ent_input), FALSE, TRUE, 0);
	
	gtk_widget_show_all (dia_input);

	result = gtk_dialog_run (GTK_DIALOG(dia_input));
	switch (result) {
		case GTK_RESPONSE_ACCEPT: 
			response = strdup((char *)gtk_entry_get_text (GTK_ENTRY(ent_input)));
			break;
		default:
			break;
	}
	gtk_widget_destroy (dia_input);

	return (response);
}

/****************************************************************************
 * List handling functions
 ****************************************************************************/
void list_columns_set (list_ *list, int nr_of_cols, char *titles[]) {
	int i;
	GType *types = NULL;
	GtkCellRenderer *renderer = NULL;
	GList *old_cols = NULL;
	GList *list_iter = NULL;
	
	/* Dettach the treemodel from the view and discard it */
	gtk_tree_view_set_model(list->treeview, NULL);

	/* Clear columns in tree view */
	old_cols = gtk_tree_view_get_columns(list->treeview);
	list_iter = old_cols;
	while (list_iter) {
		gtk_tree_view_remove_column(list->treeview, list_iter->data);
		list_iter = g_list_next (list_iter);
	}
	g_list_free (old_cols);

	/* Create new Treeview columns */
	for (i = 0; i < nr_of_cols; i++) {
//		list_column_add (list, titles[i]);
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer, "editable", TRUE, NULL);
//		g_object_set(renderer, "mode", GTK_CELL_RENDERER_SELECTED, NULL);
		gtk_tree_view_insert_column_with_attributes (
				GTK_TREE_VIEW(list->treeview),
				-1,
				titles[i],
				renderer,
				"text", i,
				NULL);
		g_signal_connect (renderer, "edited", (GCallback) ui_cell_edited_cb, NULL);
	}
	
	/* Create new Liststore from columns */
	types = malloc(sizeof(GType) * nr_of_cols);
	for (i = 0; i < nr_of_cols; i++) {
		types[i] = G_TYPE_STRING;
	}
	list->liststore = gtk_list_store_newv (nr_of_cols, types);
	gtk_list_store_set_column_types (list->liststore, nr_of_cols, types);

	/* Re-attach the model to the view */
	gtk_tree_view_set_model (list->treeview, GTK_TREE_MODEL(list->liststore));
	g_object_unref (list->liststore); /* Garbage collection */

	return;
}

void list_column_add (list_ *list, char *title) {
	GList *columns = NULL;
	int nr_of_cols = 0;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *col;
	GType *types = NULL;
	
	columns = gtk_tree_view_get_columns(list->treeview);
	nr_of_cols = g_list_length(columns);
	g_list_free (columns);
	
	renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "editable", TRUE, NULL);
	col = gtk_tree_view_column_new_with_attributes(
			title, 
			renderer,
			NULL);
	g_signal_connect (renderer, "edited", (GCallback) ui_cell_edited_cb, NULL);

	gtk_tree_view_column_set_resizable (GTK_TREE_VIEW_COLUMN(col), TRUE);
	gtk_tree_view_column_set_reorderable (GTK_TREE_VIEW_COLUMN(col), TRUE);

	gtk_tree_view_append_column (list->treeview, col);

	/* Here we build a whole new liststore with the new column because
	 * we can't simply add an column to the liststore. */
	types = malloc(sizeof(GType) * (nr_of_cols + 1));
	for (i = 0; i < (nr_of_cols + 1); i++) {
		types[i] = G_TYPE_STRING;
	}
	list->liststore = gtk_list_store_newv (nr_of_cols, types);
	gtk_list_store_set_column_types (list->liststore, nr_of_cols + 1, types);

	
}

void list_column_delete (list_ *list) {
	;
}

void list_row_add (list_ *list, int nr_of_cols, char *values[]) {
	GtkTreeIter treeiter;
	int i;

	gtk_list_store_append (list->liststore, &treeiter);
	for (i = 0; i < nr_of_cols; i++) {
		gtk_list_store_set (list->liststore, &treeiter, i, values[i], -1);
	}
}

list_ *list_create (void) {
	list_ *list = NULL;
	GtkTreeIter treeiter;
	GtkTreeSelection *treeselection;
	list = malloc(sizeof(list_));

	list->treeview  = GTK_TREE_VIEW (gtk_tree_view_new());
	list_column_add (list, "Column A");
	
	treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW(list->treeview));
	gtk_tree_selection_set_mode (treeselection, GTK_SELECTION_NONE);
	
	list->liststore = GTK_LIST_STORE(gtk_list_store_new(1, G_TYPE_STRING));
	gtk_list_store_append (list->liststore, &treeiter);
	gtk_list_store_set (list->liststore, &treeiter,
			0, "Row1",
			-1);
	
	gtk_tree_view_set_model (list->treeview, GTK_TREE_MODEL(list->liststore));

	return (list);
}

/****************************************************************************
 * Callbacks 
 ****************************************************************************/
void ui_menu_file_open_cb (void) {
		GtkWidget *win_file_open;
	
	win_file_open = gtk_file_selection_new ("Open file");
	
//    g_signal_connect (
//			G_OBJECT (GTK_FILE_SELECTION (win_file_open)->ok_button),
//			"clicked", 
//			G_CALLBACK (ui_file_open_btn_ok_cb), 
//			(gpointer) win_file_open);
//    g_signal_connect_swapped (
//			G_OBJECT (GTK_FILE_SELECTION (win_file_open)->cancel_button),
//			"clicked", 
//			G_CALLBACK (gtk_widget_destroy), 
//			G_OBJECT (win_file_open));
	
	gtk_widget_show (win_file_open);
}

void ui_menu_column_add_cb (void) {
	char *column_name = NULL;
	
	column_name = gtk_input_dialog ("Enter the column name", "Col");
	if (column_name) {
		list_column_add (list, column_name);
		free (column_name);
	}
}
void ui_menu_column_rename_cb (void) {
	char *column_name = NULL;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	
	
	column_name = gtk_input_dialog ("Enter the column name", "Col");
	if (column_name) {
		gtk_tree_view_get_cursor (list->treeview, &path, &column);
		gtk_tree_view_column_set_title (column, column_name);
		free (column_name);
	}
}


void ui_menu_column_delete_cb (void) {
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	
	gtk_tree_view_get_cursor (list->treeview, &path, &column);

	gtk_tree_view_remove_column (list->treeview, column);

	list_column_delete (list);
}

void ui_cell_edited_cb (GtkCellRendererText *cell, gchar *path_string, gchar *new_text, gpointer *data) {
	GList *columns, *list_iter;
	GtkTreePath *path;
	GtkTreeViewColumn *column;
	gchar *old_text;
	GtkTreeIter iter;
	int col, i;
	
	/* Get column number */
	printf ("a\n");
	gtk_tree_view_get_cursor (list->treeview, &path, &column);
	path = gtk_tree_path_new_from_string (path_string);
	columns = gtk_tree_view_get_columns (list->treeview);
	list_iter = columns;
	for (i = 0; i < g_list_length(columns); i++) {
		if (list_iter->data == column) {
			col = i;
			break;
		}
		list_iter = g_list_next (list_iter);
	}
	g_list_free (columns);
	
	/* Set new text */
	printf ("b\n");
	gtk_tree_model_get_iter (GTK_TREE_MODEL(list->liststore), &iter, path);

	gtk_tree_model_row_changed (list->liststore, path, &iter);
	
	printf ("c\n");
	gtk_tree_model_get (GTK_TREE_MODEL(list->liststore), &iter, col, &old_text, -1);
	if (old_text != NULL) {
		g_free (old_text);
	}
	
	printf ("d\n");
	gtk_list_store_set (GTK_LIST_STORE(list->liststore), &iter, col, new_text, -1);
}


/****************************************************************************
 * User interface creation functions
 ****************************************************************************/
GtkWidget *ui_create_menubar (GtkWidget *window) {
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	
	accel_group = gtk_accel_group_new ();
	
	item_factory = gtk_item_factory_new (GTK_TYPE_MENU_BAR, "<main>", accel_group);
	
	gtk_item_factory_create_items (item_factory, ui_nmenu_items, ui_menu_items, NULL);
	
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	
	return gtk_item_factory_get_widget (item_factory, "<main>");
}


/****************************************************************************
 * Main
 ****************************************************************************/
int main (int argc, char *argv[]) {
	GtkWidget *win_main;
	GtkWidget *vbox_main;

	char *cols[] = {"aa", "bb"};
	char *vals[] = {"CC", "DD"};
	
	gtk_init (&argc, &argv);
	g_type_init();

	win_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW(win_main), 400, 200);
	
	vbox_main = gtk_vbox_new (FALSE, 2);

	list = list_create();
	list_columns_set (list, 2, cols);
	list_row_add (list, 2, vals);
	list_row_add (list, 2, vals);
	list_row_add (list, 2, vals);

	gtk_box_pack_start (GTK_BOX(vbox_main), GTK_WIDGET(ui_create_menubar(win_main)), FALSE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox_main), GTK_WIDGET(list->treeview), FALSE, TRUE, 0);

	gtk_container_add (GTK_CONTAINER(win_main), vbox_main);

	gtk_widget_show_all (win_main);
	gtk_main();
	
	return (0);
}
