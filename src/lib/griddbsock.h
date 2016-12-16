#ifndef _SNAFU_GRIDDB_SOCKET_H_
#define _SNAFU_GRIDDB_SOCKET_H_

#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */

#define GRIDDBSOCKET_DIR  "/var/local/gridhost"
#define GRIDDBSOCKET_USER "localhost"
#define GRIDDBSOCKET_BASE "/griddb"
#define LISTEN_BACKLOG 50

struct sockaddr_un *griddb_generate_sockaddr( void );


#endif /*!_SNAFU_GRIDDB_SOCKET_H_*/
