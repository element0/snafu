/*
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 */

/**
 * @file
 * Griddb Client
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	/* NAME_MAX, PATH_MAX, fsync(), fdatasync() */
#include <stdarg.h>	/* va_list, va_start(), va_arg(), va_end() */

#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */

#include "griddbsock.h"
#include "griddbmsg.h"
#include "griddb_client.h"
#include "buflink.h"

struct griddb_handle *griddb_open () {
	struct griddb_handle *g = malloc(sizeof(struct griddb_handle));
	if( g == NULL ) {
		fprintf( stderr, "can't malloc griddb handle\n");
		return NULL;
	}
	memset(g,0,sizeof(struct griddb_handle));

	g->sa = griddb_generate_sockaddr();

	return g;
}

void griddb_close ( struct griddb_handle *g ) {
	if( g == NULL ) return;
	if( g->sa != NULL ) free( g->sa );
	free( g );
}

int griddbmsg_send( struct griddb_handle *g, void *buf, void **rbuf ) {
/*
	char *msgbase;

	if( argc == 2 ) {
		msgbase = argv[1];
	}else{
		msgbase = "string set -keyno=12 -valstr=redboy";
	}
*/
	if( g == NULL) {
		fprintf(stderr, "griddb_client: griddb handle is NULL.\n");
		return -1;
	}

	struct sockaddr_un *sa = g->sa;

	int soc = socket(AF_UNIX, SOCK_STREAM, 0);
	if( soc == -1 ) {
		perror("grid_get_socket: socket");
		return -1;
	}
	if( connect(soc, (struct sockaddr *) sa,
				sizeof(struct sockaddr_un)) == -1 ) {
		perror("grid_get_socket: connect");
		return -1;
	}


	/* SEND MSG */
	struct buflink *cur = buf;
	while(cur != NULL) {
		if( write(soc, cur, cur->len) != cur->len ) {
			fprintf(stderr, "partial/failed write.\n");
		}
		cur = cur->next;
	}
	fsync(soc);
	
	struct buflink *reply_handle = malloc(sizeof(struct buflink));
	if( reply_handle == NULL ) {
		fprintf(stderr, "gridmsg_send: can't malloc reply buffer.\n");
		return -1;
	}
	memset(reply_handle, 0, sizeof(struct buflink));

	struct buflink *cur = reply_handle;
	while(cur != NULL) {
		cur->len = read(soc, (void *)cur->c, GRIDMSGBUFSIZE);
		if( cur->len == GRIDMSGBUFSIZE ) {
			cur->next = malloc(sizeof(struct buflink));
		}else if( cur->len == -1 ) {
			perror("gridmsg_send: read");
			break;
		}
		cur = cur->next;
	}



	/* PRINT RESULT */
	if( reply_handle == NULL ) {
		fprintf(stderr, "test_griddb: socket returned NULL.\n");
	}

	// this should be done inside griddb handle
	close(soc);

	*rbuf = reply_handle;
	return 0;
}

void *griddbmsg( void *g, char *base, ... ) {

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
	if( griddbmsg_send( g, buf, &reply ) == -1 ) {
		fprintf(stderr, "gridmsg: gridmsg_send() failed.\n");
		return NULL;
	}
	buflink_cleanup(buf);

	return reply;
}



