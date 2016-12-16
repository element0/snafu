#include <stdio.h>
#include <errno.h>

#include "../src/lib/gridclient.h"
#include "../src/lib/gridsock.h"
#include "../src/lib/gridmsg.h"

int main ( int argc, char *argv[] ) {
	
	/* grid */

	struct cosmos_grid *g = grid_open( NULL );
	if( g == NULL ) {
		fprintf(stderr, "test_gridclient: grid is NULL.\n");
		return -1;
	}

	/* gridmsg */

	struct gridmsg *msg = gridmsg_new( g );
	if( msg == NULL ) {
		fprintf(stderr, "test_gridclient: msg is NULL.\n");
		grid_close( g );
		return -1;
	}

	/* set message's aframe */

	if( gridmsg_set_aframestr( msg, "griddb" ) == -1 ) goto abort_msg;

	/* add a command to message */

	if( gridmsg_set_command( msg, "get" ) == -1 ) goto abort_msg;

	/* push argument to message */

	if( gridmsg_push_arg( msg, "aframeno=" ) == -1 ) goto abort_msg;

	/* append argument to message */

	char itoa[25];
	snprintf( itoa, 25, "%lu", (unsigned long)1 );
	if( gridmsg_append_str( msg, itoa ) == -1 ) goto abort_msg;


	/* push another argument */

	if( gridmsg_push_arg( msg, "color=" ) == -1 ) goto abort_msg;
	if( gridmsg_append_str( msg, "blue" ) == -1 ) goto abort_msg;

	/* print message */

	printf("gridmsg: \"");
	fflush(stdout);
	gridmsg_print( msg );
	printf("\"\n");


	/* change the message's aframe */

	if( gridmsg_set_aframestr( msg, "/usr/var/run" ) == -1 )
		goto abort_msg;

	/* print message */

	printf("gridmsg: \"");
	fflush(stdout);
	gridmsg_print( msg );
	printf("\"\n");

	/* send message */

	void *resbuf;
	size_t reslen;
	if( gridclient_send_msg( msg, &resbuf, &reslen ) == -1 )
		goto abort_msg;

	/* demo result should be an aframe */

	struct cosmos_aframe *a;
	if( aframe_at_buffer( &a, resbuf, reslen ) == -1 )
		goto abort_msg;
	
	/* check reported errors */
	switch( aframe_get_error( a ) ) {
		case 0:
			break; /* no error, proceed */
		case ENOENT:
			goto abort_aframe;
	}
	struct stat sbuf;
	if( aframe_stat( a, &sbuf ) == -1 )
		goto abort_aframe;

	printf("aframe_stat returned ino: %ld\n", sbuf.st_ino);



abort_aframe:
	aframe_cleanup( a );
abort_msg:
	gridmsg_cleanup(msg);



	struct sockaddr_un *sa = grid_generate_sockaddr();

	int sfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if((connect(sfd, (struct sockaddr *) sa, sizeof(struct sockaddr_un))) != -1 )
		write(sfd, "hello", 6);

	/* success */
	grid_close( g );

	return 0;
}
