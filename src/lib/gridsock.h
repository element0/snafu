#ifndef _LIBGRID_GRIDSOCK_H_
#define _LIBGRID_GRIDSOCK_H_

/**
 * @file
 *
 * This is the Interface for the Socket to the grid.
 * Included by both gridclient and gridhost.
 *
 */
#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */


#define GRIDHOSTSOCKET_DIR  "/var/local/gridhost"
#define GRIDHOSTSOCKET_USER "raygan"
#define GRIDHOSTSOCKET_BASE "/local"


struct sockaddr_un *grid_generate_sockaddr( void );

#endif /*!_LIBGRID_GRIDSOCK_H_*/
