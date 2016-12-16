#ifndef _SNAFU_GRIDDB_MESSAGE_H_
#define _SNAFU_GRIDDB_MESSAGE_H_
/*
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 */

/**
 * @file
 * Sender of Griddb Messages as part of Griddb Client.
 */


#include "cosmos_grid.h"

void * griddb_msg( struct cosmos_grid *g, int protno, char dbid, char cmd,
		   char *ksiz, char sepcode1, char *vsiz, char sepcode2,
		   char *key, char *val );

#endif /*!_SNAFU_GRIDDB_MESSAGE_H_
