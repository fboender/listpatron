/****************************************************************************
 *
 * ListPatron - debug
 *
 * Debugging stuff
 *
 * Copyright (C), 2004 Ferry Boender. Released under the General Public License
 * For more information, see the COPYING file supplied with this program.                                                          
 * 
 ****************************************************************************/

#include "config.h"

#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "debug.h"

extern int opt_verbose;

void debug_msg(int dbg_type, char *file, int line, char *fmt, ...) {
#ifdef DEBUG
	va_list argp;
	char *err_usr = NULL, *err;
	int n = 10, err_len = 10;
	
	if (!opt_verbose) { 
		return; 
	}

	err_usr = malloc(err_len);

	while (n == err_len) { /* Keep trying until msg fits in the buffer */
		va_start(argp, fmt);
		n = vsnprintf(err_usr, err_len, fmt, argp);
		va_end(argp);
		
		if (n < -1) {
			return;
		} else 
		if (n >= err_len) { /* Throw some more mem at the buf */
			err_len = (2 * err_len);
			n = err_len;
			err_usr = realloc(err_usr, err_len+1);
		} else {
			n = 0; /* That'll be enough, thank you */
		}
	}
	
	err = malloc(sizeof(char) * (12 + strlen(file) + 8 + strlen(err_usr) + 2));
	sprintf(err, "DEBUG: %s:%i : %s\n", file, line, err_usr);
	fprintf(stderr, err);
	free(err);

	free(err_usr);
#endif /* DEBUG */
}

