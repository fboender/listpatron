/****************************************************************************
 * New Menu
 ****************************************************************************/

/* Normal items */
static GtkActionEntry entries[] = {
	{ "FileMenu"      , NULL            , "_File" }              , 
	{ "New"           , GTK_STOCK_NEW   , "_New"                 , NULL         , "Create a new file"                           , ui_menu_file_new_cb }            , 
	{ "Open"          , GTK_STOCK_OPEN  , "_Open"                , NULL         , "Open a file"                                 , ui_menu_file_open_cb }           , 
	{ "Save"          , GTK_STOCK_SAVE  , "_Save"                , NULL         , "Save the current file"                       , ui_menu_file_save_cb }           , 
	{ "SaveAs"        , NULL            , "Save _As"             , NULL         , "Save the file under a different name"        , ui_menu_file_save_as_cb }        , 
	{ "ImportMenu"    , NULL            , "_Import" }            , 
	{ "ImportCSV"     , GTK_STOCK_OPEN  , "_Character Seperated" , ""           , "Import from character seperated values file" , ui_menu_file_import_csv_cb }     , 
	{ "ExportMenu"    , NULL            , "_Export" }            , 
	{ "ExportCSV"     , GTK_STOCK_SAVE  , "_Character Seperated" , ""           , "Export to character seperated values file"   , ui_menu_file_export_csv_cb }     , 
	{ "ExportPS"      , GTK_STOCK_SAVE  , "_PostScript"          , ""           , "Export to PostScript file"                   , ui_menu_file_export_ps_cb }      , 
	{ "ExportHTML"    , GTK_STOCK_SAVE  , "_HTML"                , ""           , "Export to a HTML file"                       , ui_menu_file_export_html_cb }    , 
	{ "Exit"          , GTK_STOCK_QUIT  , "E_xit"                , ""           , "Exit the program"                            , ui_menu_file_quit_cb }           , 

	{ "EditMenu"      , NULL            , "_Edit" }              , 
	{ "Cut"           , GTK_STOCK_CUT   , "_Cut"                 , "<control>X" , "Cut selection" }                             , 
	{ "Copy"          , GTK_STOCK_COPY  , "_Copy"                , "<control>C" , "Copy selection to the clipboard" }           , 
	{ "Paste"         , GTK_STOCK_PASTE , "_Paste"               , "<control>V" , "Paste selection from the clipboard"  }       , 
	{ "Find"          , GTK_STOCK_FIND  , "_Find"                , "<control>F" , "Find a value"                                , ui_menu_edit_find_cb }           , 

	{ "DataMenu"      , NULL            , "_Data" }              , 
	{ "ColumnMenu"    , NULL            , "_Columns" }           , 
	{ "ColAdd"        , LP_STOCK_COL_ADD, "_Add"                 , "<control>a" , "Add a column to the list"                    , ui_menu_column_add_cb }          , 
	{ "ColDel"        , LP_STOCK_COL_DEL, "_Delete"              , "<control>d" , "Delete the current column"                   , ui_menu_column_delete_cb }       , 
	{ "ColRename"     , NULL            , "_Rename"              , "<control>r" , "Rename the current column"                   , ui_menu_column_rename_cb }       , 
	{ "RowMenu"       , NULL            , "_Rows" }              , 
	{ "RowAdd"        , LP_STOCK_ROW_ADD, "_Add"                 , NULL         , "Add a row to the list"                       , ui_menu_row_add_cb }             , 
	{ "RowDel"        , LP_STOCK_ROW_DEL, "_Delete"              , NULL         , "Delete the current row"                      , ui_menu_row_delete_cb }          , 
	{ "SortMenu"      , NULL            , "_Sort" }              , 
	{ "SortEdit"      , NULL            , "_Edit sorting rules"  , NULL         , "Add, change and delete sorting rules"        , ui_menu_sort_rules_cb } , 
	{ "SortNew"       , NULL            , "_New sorting rule"    , NULL         , "Add a sorting rule"                          , ui_menu_sort_edit_cb }           , 
	{ "FilterMenu"    , NULL            , "_Filter" }            , 
	{ "ReportMenu"    , NULL            , "_Report" }            , 

#ifdef DEBUG
	{ "DebugMenu"     , NULL            , "De_bug" }             , 
	{ "AddTestData"   , NULL            , "Add test _data"       , NULL         , "Add test data to an empty list"              , ui_menu_debug_addtestdata_cb }   , 
	{ "AddTestRows"   , NULL            , "Add test _rows"       , NULL         , "Add test rows to a testdata list"            , ui_menu_debug_addtestrows_cb }   , 
	{ "DumpSortRules" , NULL            , "Dump _sorting rules"  , NULL         , "Dump the available sorting rules"            , ui_menu_debug_dumpsortrules_cb } ,
#endif /* DEBUG */

	{ "HelpMenu"      , NULL            , "_Help" }             , 
	{ "Usage"         , NULL            , "_Usage"              , NULL         , "Quick usage information"                      , ui_menu_help_usage_cb}           , 
	{ "About"         , NULL            , "_About"              , NULL         , "About ListPatron"                             , ui_menu_help_about_cb }          , 
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='New'/>"
"      <menuitem action='Open'/>"
"      <menuitem action='Save'/>"
"      <menuitem action='SaveAs'/>"
"      <menu action='ImportMenu'>"
"        <menuitem action='ImportCSV' />"
"      </menu>"
"      <menu action='ExportMenu'>"
"        <menuitem action='ExportCSV' />"
"        <menuitem action='ExportPS' />"
"        <menuitem action='ExportHTML' />"
"      </menu>"
"      <menuitem action='Exit'/>"
"    </menu>"
"    <menu action='EditMenu'>"
"      <menuitem action='Cut'/>"
"      <menuitem action='Copy'/>"
"      <menuitem action='Paste'/>"
"      <separator/>"
"      <menuitem action='Find'/>"
"    </menu>"
"    <menu action='DataMenu'>"
"      <menu action='ColumnMenu'>"
"        <menuitem action='ColAdd'/>"
"        <menuitem action='ColDel'/>"
"        <menuitem action='ColRename'/>"
"      </menu>"
"      <menu action='RowMenu'>"
"        <menuitem action='RowAdd'/>"
"        <menuitem action='RowDel'/>"
"      </menu>"
"      <separator/>"
"      <menu action='SortMenu'>"
"        <menuitem action='SortEdit'/>"
"        <menuitem action='SortNew'/>"
"        <separator/>"
"      </menu>"
"      <menu action='FilterMenu'>"
"      </menu>"
"      <menu action='ReportMenu'>"
"      </menu>"
"    </menu>"
#ifdef DEBUG
"    <menu action='DebugMenu'>"
"      <menuitem action='AddTestData'/>"
"      <menuitem action='AddTestRows'/>"
"      <separator/>"
"      <menuitem action='DumpSortRules'/>"
"    </menu>"
#endif /* DEBUG */
"    <menu action='HelpMenu'>"
"      <menuitem action='Usage'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static const char *ui_description_toolbar =
"<ui>"
"  <toolbar name='ToolbarFile'>"
"    <toolitem action='New'/>"
"    <toolitem action='Open'/>"
"    <toolitem action='Save'/>"
"    <toolitem action='Exit'/>"
"    <separator/>"
"    <toolitem action='Cut'/>"
"    <toolitem action='Copy'/>"
"    <toolitem action='Paste'/>"
"    <toolitem action='Find'/>"
"    <separator/>"
"    <toolitem action='ColAdd'/>"
"    <toolitem action='ColDel'/>"
"    <separator/>"
"    <toolitem action='RowAdd'/>"
"    <toolitem action='RowDel'/>"
"  </toolbar>"
"</ui>";


