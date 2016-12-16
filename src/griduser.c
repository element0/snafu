/*
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 */


/** @file
 * (Consider namechange to Griduserhost)
 * (Consider splitting Griduserhost and Gridguest to two programs)
 *
 * "Griduserhost" is a mouthfull.  But it's all that...and more.
 *
 * Griduserhost runs in two different modes:
 * - server mode
 * - remote guest mode
 *
 * The "server mode" program runs as a daemon on the localhost.
 * server mode must be launched before snafu FUSE volume can mount.
 *
 * The "remote guest mode" program is launched via ssh by the a "server mode"
 * program on a remote machine
 * and pipes back to "server mode" daemon on local machine.
 * 
 * (The grid consists of ssh connections between machines.)
 *
 * One host may have one `griduserhost` daemon running for each user,
 * also, a griduser running as a client process for each connected gridnode.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "lib/cosmos_grid.h"	/* struct cosmos_grid */
#include "lib/gridsock.h"	/* grid_generate_sockaddr() */
#include "lib/gridclient.h"	/* grid_open() */
#include "lib/griddb_client.h"	/* griddb_open() */

#include "lib/gridmsg_engine.h" /* gridmsg_exec() */

#define handle_error(msg) \
	do{ perror(msg); exit(EXIT_FAILURE); } while (0)


/******************************************
 *         GRIDUSERHOST CONFIG            *
 ******************************************/

#define LISTEN_BACKLOG 50

#define GRIDHOST_INSTALL_PREFIX	/usr/local
#define GRIDHOST_INSTALL_LIBPATH GRIDHOST_INSTALL_PREFIX/lib/snafu

#define GRIDHOST_SERVERMOD_LIBPATH GRIDHOST_INSTALL_LIBPATH/server
#define GRIDHOST_LOOKUPMOD_LIBPATH GRIDHOST_INSTALL_LIBPATH/lookup




/******************************************
 *     GRIDUSERHOST INTERNAL FUNCTIONS    *
 ******************************************/

/**
 * this function can be called anytime to reload server modules
 */
static void *griduserhost_load_servermods() {
	return NULL;
}

/**
 * this function can be called anytime to reload lookupmod
 */
static void *griduserhost_load_lookupmod() {
	return NULL;
}

/**
 * Load server modules.
 * Load lookup module.
 */
static struct griduserhost *griduserhost_init() {
	struct griduserhost *guh = malloc(sizeof(struct griduserhost));
	if( guh == NULL ) return NULL;
	memset(guh, 0, sizeof(*guh));

	guh->server = griduserhost_load_servermods();
	guh->lookup = griduserhost_load_lookupmod();

	return guh;
}



/**
 * usage: griduserhost
 *
 * Main loop should
 * - accept new connections
 * - purge stale connections
 * - take turns with open connections
 * - poll waiting grid `delegation frames`
 * - dynamic reload of modules
 * - handle signals
 */
int main(int argc, char *argv[]){
	if( argc != 1 ) {
		printf("usage: griduserhost\n");
		return -1;
	}

	int retval = -1;
	char *errstr = "completed successfully.";

	/* socket addresses created in grid_open and griddb_open */
	struct cosmos_grid *g = grid_open();
	if(g == NULL) {
		errstr = "grid_open failed.\n";
		goto abort_grid;
	}

	struct griddb_handle *gdb = griddb_open();
	if(gdb == NULL) {
		errstr = "griddb_open failed.\n";
		goto abort_griddb;
	}

	struct griduserhost *guh = griduserhost_init();
	if(guh == NULL) {
		errstr = "griduserhost_init failed.\n";
		goto abort_griduserhost;
	}

	/* open socket */
	int sfd;
	sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if( sfd == -1 )
		handle_error("socket");

	/* bind */
	if( bind( sfd, (struct sockaddr *) g->sockaddr,
				sizeof(struct sockaddr_un)) == -1)
		handle_error("bind");

	/* listen */

	if( listen(sfd, LISTEN_BACKLOG) == -1 )
		handle_error("listen");

	/* accept loop */

	int cfd;
	struct sockaddr_un peer_addr;
	socklen_t peer_addr_size;

	while ( (cfd = accept( sfd, (struct sockaddr *) &peer_addr, &peer_addr_size)) != -1 )
	{

		/* read msg */

		struct buflink *buf = malloc(sizeof(struct buflink));
		if( buf == NULL )
			handle_error("can't malloc input buffer.\n");
		memset(buf, 0, sizeof(struct buflink));

		struct buflink *cur = buf;
		while( cur != NULL ) {
			cur->len = read(cfd, (void *)cur->c, GRIDMSGBUFSIZE-1);
			if( cur->len == GRIDMSGBUFSIZE-1 ) {
				cur->next = malloc(sizeof(struct buflink));
			}
			cur = cur->next;
		}


		/* interpret and execute */

		struct buflink *rbuf = gridmsg_exec( gdb, buf );

		buflink_cleanup( buf );


		/* write result */

		cur = rbuf;
		while( cur != NULL ) {
			write( cfd, (void *)cur, cur->len );
			cur = cur->next;
		}

		buflink_cleanup( rbuf );

		close(cfd);
	}
	close(sfd);

	retval = 0;
abort:
	griduserhost_cleanup( guh );
abort_griduserhost:
	griddb_close( gdb );
abort_griddb:
	grid_close( g );
abort_grid:
	fprintf(stderr, "griduserhost: %s\n", errstr);
	return retval;
}
