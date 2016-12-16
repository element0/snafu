/*
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 */

/**
 * @file
 * Sender of Griddb Messages as part of Griddb Client.
 */
#include <stdio.h>

#include "cosmos_grid.h"
#include "griddbmsg.h"

/**
 * Send Griddb Message
 */
void * griddb_msg( struct cosmos_grid *g, int protno, char dbid, char cmd,
		   char *ksiz, char sepcode1, char *vsiz, char sepcode2,
		   char *key, char *val ) {

	printf("%d%c%c%s%c%s%c%s%s\n",
			protno,
			dbid,
			cmd,
			ksiz,
			sepcode1,
			vsiz,
			sepcode2,
			key,
			val );

	return NULL;
}
