#ifndef _SNAFU_GRIDDB_CLIENT_H_
#define _SNAFU_GRIDDB_CLIENT_H_

#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */

struct griddb_handle {
	struct sockaddr_un *sa;
};

struct griddb_handle *griddb_open();
void                  griddb_close( struct griddb_handle *g );

int   griddbmsg_send( struct griddb_handle *g, void *buf, void **rbuf );
void *griddbmsg( void *g, char *base, ... );


#endif /*!_SNAFU_GRIDDB_CLIENT_H_*/
