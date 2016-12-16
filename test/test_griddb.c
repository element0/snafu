#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>	/* NAME_MAX, PATH_MAX, fsync(), fdatasync() */
#include <stdarg.h>	/* va_list, va_start(), va_arg(), va_end() */

#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */

#include "../src/lib/griddb.h"
#include "../src/lib/buflink.h"

int main( int argc, char *argv[] ) {

	char *msgbase;

	if( argc == 2 ) {
		msgbase = argv[1];
	}else{
		msgbase = "string set -keyno=12 -valstr=redboy";
	}

	struct sockaddr_un *sa = griddb_generate_sockaddr( );

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
	if( write(soc, msgbase, strlen(msgbase)) != strlen(msgbase) ) {
		fprintf(stderr, "partial/failed write.\n");
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
	}else{
		buflink_print( reply_handle );
		printf("\n");
	}

	buflink_cleanup( reply_handle);

	close(soc);
	free(sa);
	return 0;
}
