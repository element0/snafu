#ifndef _COSMOS_GRIDDB_H_
#define _COSMOS_GRIDDB_H_

/**
 * @file
 * 
 * The `griddb` daemon serves primatives to Griduser Daemons.
 * Should run as a trusted user or in a container.
 */
#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */

#include <kclangc.h>

/**
 * first draft defines one database per letter of the english alphabet.
 * these are created at access time.
 */
#define GRIDDB_NUMDB 26

/**
 * The snafu file system primatives databases.
 */
struct griddb {
	void *db[GRIDDB_NUMDB];
	int vers;
};

/**
 * inter return msg handle
 */
struct griddb_rmsg {
	char code[2];
	char *val;
	size_t vlen;
	void (*cleanfn)( void * );
};


/**
 * init the griddb databases if needed before access
 */
struct griddb * griddb_init_databases( void );

/**
 * cleanup the griddb
 */
void griddb_cleanup( struct griddb * g );

/**
 * the main server loop
 */
void   griddb_main_loop( struct griddb *grdb );

#endif /*!_COSMOS_GRIDDB_H_*/
