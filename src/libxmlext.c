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

#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "libxmlext.h"

xmlNodeSet *xml_get_nodeset(xmlDocPtr doc, char *xpath){
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;

	context = xmlXPathNewContext(doc);
	result = xmlXPathEvalExpression(xpath, context);
	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
		return NULL;
	}
	xmlXPathFreeContext(context);

	return (result->nodesetval);
}

/* Return value exists only during existance of xml document */
char *xml_get_element_content(xmlDocPtr doc, char *xpath) {
	xmlXPathContextPtr context;
	xmlXPathObjectPtr result;
	
	context = xmlXPathNewContext(doc);
	result = xmlXPathEvalExpression(xpath, context);
	if (xmlXPathNodeSetIsEmpty(result->nodesetval)) {
		return ("");
	}
	xmlXPathFreeContext(context);

	if (result->nodesetval->nodeTab[0]->children) {
		return (result->nodesetval->nodeTab[0]->children->content);
	} else {
		return ("");
	}
}

xmlNodePtr xml_add_element_content(xmlNodePtr node_parent, char *element_name, char *fmt, ...) {
	va_list argp;
	char *content = NULL;
	xmlNodePtr node_new = NULL;
	int n = 10, content_len = 10;
	
	content = malloc(content_len);

	/* I'd like to unify this into a single function, but it seems that can't 
	 * be done. I'm getting a '`va_start' used in function with fixed args'
	 * msgor. If anyone knows, please mail me */
	while (n == content_len) { /* Keep trying until content fits in the buffer */
		va_start(argp, fmt);
		n = vsnprintf(content, content_len, fmt, argp);
		va_end(argp);
		
		if (n < -1) {
			return (NULL);
		} else 
		if (n >= content_len) { /* Throw some more mem at the buf */
			content_len = (2 * content_len);
			n = content_len;
			content= realloc(content, content_len+1);
		} else {
			n = 0; /* That'll be enough, thank you */
		}
	}
	
	node_new = xmlNewChild(node_parent, NULL, element_name, (const xmlChar *)content);
	
	free(content);

	return (node_new);
}

/* NOTICE: Unused */
char *xml_element_get_value(xmlNodePtr node_parent, char *element_name) {
	if (node_parent->type == XML_ELEMENT_NODE) {
		if (strcmp(node_parent->name, element_name) == 0) {
			return (node_parent->children->content);
		}
	}

	return (NULL);
}


