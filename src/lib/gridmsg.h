#ifndef _LIBGRID_GRIDMSG_H_
#define _LIBGRID_GRIDMSG_H_

/** 
 * @file
 *
 * Replacing "Grid Instruction" with "Grid Message"
 *
 * Grid Messages are passed between hosts and for distributed processing.
 *
 * Grid Messages are also sent between Gridclient Library and Griduser Daemon.
 *
 * Hiena purposes the Grid Message as a Production Instruction for
 * producing Domain Cells from various methods.  The hiena Producer Functions
 * can be run by the Production Instructions.
 *
 * Grid Messages as a category can apply a user extensible vocabulary of 
 * functions.
 *
 * Gridmsg will be written to a stream socket.
 *
 */

#include "cosmos_grid.h"

#define GRIDMSGBUFSIZE 1024
#include "buflink.h"

/**
 * Format and send Gridmsg between Gridclient and Griduser Daemon
 *
 * Accepts variable arguments like `printf()`.
 * Composes message into one or more `struct buflink`.
 *
 * Currently implements `%s` and `%lu`.
 *
 * @param g grid handle
 * @param base format string with % replacements
 * @param ... variable arguments
 * @return pointer to anything returned by the grid. use wisely.
 */
void *gridmsg( struct cosmos_grid *g, char *base, ... );



/**
 * Experimental "Grid Instruction" structure,
 * AKA Hiena Production Instruction
 *
 * This might not last.
 */
struct grid_instruction {
	char *cmdline;		/* if commandline invoked the instruction */
	size_t cmdlinelen;
	char *cmdpath;		/* if command is external util */
	size_t cmdpathlen;
	void *builtin;		/* if command is gridhost builtin */
	int    argc;
	char **args;
};

#endif /*!_LIBGRID_GRIDMSG_H_*/
