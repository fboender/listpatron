/****************************************************************************
 *
 * ListPatron - libmxlext
 *
 * Extensions to lib xml for easier handling of certain data
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#ifndef LIBXMLEXT_H
#define LIBXMLEXT_H

xmlNodeSet *xml_get_nodeset(xmlDocPtr doc, char *xpath);
char *xml_get_element_content(xmlDocPtr doc, char *xpath);
xmlNodePtr xml_add_element_content(xmlNodePtr node_parent, char *element_name, char *fmt, ...);

#endif

