/*
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 */


/**
 * @file
 * 
 * The `griddb` daemon serves primatives to Griduser Daemons.
 * Should run as a trusted user or in a container.
 */

#define _GNU_SOURCE	/* required by 'struct cred' in <sys/socket.h>*/

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */


#include "lib/griddb.h"


/**
 * TODO: add a check to make sure no other instances of griddb are running.
 */
int main(int argc, char *argv[]){

	struct griddb *g = griddb_init_databases();
	
	griddb_main_loop( g );

	griddb_cleanup( g );

	return 0;
}
