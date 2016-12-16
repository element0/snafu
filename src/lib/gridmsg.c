/**
 * @file
 *
 * Implementation of Grid Message Object
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	/* NAME_MAX, PATH_MAX, fsync(), fdatasync() */
#include <stdarg.h>	/* va_list, va_start(), va_arg(), va_end() */

#include <sys/types.h>		/* recommended for portable socket() */
#include <sys/socket.h>		/* socket() */
#include <sys/un.h>		/* unix domain socket support */

#include "cosmos_grid.h"
#include "gridmsg.h"


int grid_get_socket( struct cosmos_grid *g ) {
	if( g == NULL ) {
		fprintf(stderr, "grid_get_socket: grid is NULL.\n");
		return -1;
	}
	int soc = socket(AF_UNIX, SOCK_STREAM, 0);
	if( soc == -1 ) {
		perror("grid_get_socket: socket");
		return -1;
	}
	if( connect(soc, (struct sockaddr *)g->sockaddr,
				sizeof(struct sockaddr_un)) == -1 ) {
		perror("grid_get_socket: connect");
		return -1;
	}
	return soc;
}

int grid_release_socket( struct cosmos_grid *g, int soc ) {
	close(soc);
	return 0;
}


/**
 * Sends message to grid.
 *
 * 1) Grab socket connection.
 * 2) Send
 * 3) Receive
 * 4) Release socket connection.
 */

int gridmsg_send( struct cosmos_grid *g, struct buflink *b, struct buflink **reply ) {
	if( g == NULL) return -1;
	if( b == NULL) return -1;


	int soc = grid_get_socket( g );
	if( soc == -1 ) {
		fprintf(stderr, "gridmsg_send: couldn't get socket.\n");
		return -1;
	}

	struct buflink *cur = b;

	while(cur != NULL) {
		if( write(soc, cur, cur->len) != cur->len ) {
			fprintf(stderr, "gridmsg_send: partial/failed write.\n");
			break;
		}
		cur = cur->next;
	}
	
	struct buflink *reply_handle = malloc(sizeof(struct buflink));
	if( reply_handle == NULL ) {
		fprintf(stderr, "gridmsg_send: can't malloc reply buffer.\n");
		return -1;
	}
	memset(reply_handle, 0, sizeof(struct buflink));

	cur = reply_handle;
	while(cur != NULL) {
		cur->len = read(soc, (void *)cur, GRIDMSGBUFSIZE);
		if( cur->len == GRIDMSGBUFSIZE ) {
			cur->next = malloc(sizeof(struct buflink));
		}else if( cur->len == -1 ) {
			perror("gridmsg_send: read");
			break;
		}
		cur = cur->next;
	}

	grid_release_socket( g, soc );
	*reply = reply_handle;
	return 0;
}

void *gridmsg( struct cosmos_grid *g, char *base, ... ) {

	struct buflink *buf = malloc(sizeof(*buf));
	if( buf == NULL ) {
		fprintf(stderr, "gridmsg: can't allocate buffer.\n");
		return NULL;
	}
	memset(buf,0,sizeof(*buf));

	char *c = buf->c;

	char *str;
	unsigned long ul;
	char itoa[32];

	va_list ap;
	va_start(ap, base);

	int i,j;
	i=0;
	for(j=0; j<=strlen(base); j++) {
		if(base[j] == '%') {
			j++;
			switch(base[j]) {
				case 's':
					str = va_arg(ap, char *);
					strncpy(&c[i],str,strlen(str));
					i+=strlen(str);
					break;
				case 'l':
					j++;
					if(base[j] == 'u') {
						ul = va_arg(ap,
							unsigned long);
						sprintf(itoa, "%lu", ul);
						strncpy(&c[i],itoa,
							strlen(itoa));
						i+=strlen(itoa);
					}
					break;
				default:
					break;
			}
		}else{
			c[i++] = base[j];
		}
	}
	c[i]='\0';
	buf->len=i;

	va_end(ap);

	/* gridmsg buffer ready to send */

	struct buflink *reply;
	if( gridmsg_send( g, buf, &reply ) == -1 ) {
		fprintf(stderr, "gridmsg: gridmsg_send() failed.\n");
		return NULL;
	}
	buflink_cleanup(buf);

	return reply;
}

