#ifndef _LIBGRID_COSMOS_GRID_H_
#define _LIBGRID_COSMOS_GRID_H_

/**
 * @file
 *
 */
#include <sys/socket.h>
#include "aframe.h"

struct cosmos_grid {
	/* used by gridclient and griduserhost */
	struct sockaddr_un *sockaddr;
	socklen_t sockaddrlen;

	/* used by griduser host */
	void *host;

	/* outdated... */
	void *griduser;
	int socketfd;
	struct cosmos_aframe *aframe;
};

#endif /*!_LIBGRID_COSMOS_GRID_H_*/
