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

#ifndef DEBUG_H
#define DEBUG_H

/* debug_msg levels */
#define DBG_ERR 1
#define DBG_WARN 2
#define DBG_TRACE 3
#define DBG_VERBOSE 4

void debug_msg(int dbg_type, char *file, int line, char *fmt, ...);

#endif
