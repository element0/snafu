/** @file
 * Cosmos grid implementation.
 */
#include <stdio.h>		/* fprintf() */
#include <stdlib.h>		/* malloc() */
#include <string.h>		/* memset() */
#include <limits.h>		/* LOGIN_NAME_MAX, HOST_NAME_MAX */
#include <unistd.h>
#include <pwd.h>
#include <errno.h>

#include <sys/types.h>		/* recommended for portable socket() */
#include <sys/socket.h>		/* socket() */
#include <sys/un.h>		/* unix domain socket support */

#include "gridmsg.h"
#include "gridsock.h"
#include "gridclient.h"



/**************************************
 	grid init support funcs
 **************************************/

/**
 * call this function first to acquire grid object.
 */
struct cosmos_grid *grid_open ( void ) {

	struct cosmos_grid *grid = malloc(sizeof(*grid));
	if( grid == NULL ) return NULL;

	memset(grid, 0, sizeof(*grid));

	grid->sockaddr = grid_generate_sockaddr();
	grid->sockaddrlen = sizeof(struct sockaddr_un);
	/* check /var/lib/gridhost/running socket? */
	/* no running socket? */

	/* first Proof of concept mockup doesn't have to spawn a gridhost.
	int err = gridhost_spawn( NULL );
	*/
	
	/* running socket? */

	/* the socket is stored in grid->socketfd */
	
	return grid;
}

void grid_close ( struct cosmos_grid *grid ) {
	free(grid->sockaddr);
	free(grid);
}

/*
 * Convert mountpoint's filepath to a `snafu` scheme URI.
 * ie "snafu://<user>@<host>/<absolutepath>"
 */
char *grid_filepath_to_uri ( const char *filepath, char *scheme ) {

	char *result;

	size_t sclen = strlen(scheme);
	char *username;
	size_t unlen;
	char *hostname;
	size_t hnlen;

	size_t fplen = strlen(filepath);

	
	if( gridclient_get_username( (void *)&username, &unlen ) == -1 ) {
		fprintf(stderr, "error getting username\n");
		return NULL;
	}
	if( gridclient_get_hostname( (void *)&hostname, &hnlen ) == -1 ) {
		fprintf(stderr, "error getting hostname\n");
		return NULL;
	}

	size_t tlen = sclen + 3 + unlen + 1 + hnlen + fplen + 1;

	result = malloc(sizeof(char)*tlen);
	snprintf(result, tlen, "%s://%s@%s%s",
			scheme, username, hostname, filepath);

	free(username);
	free(hostname);

	return result;
}


/**
 * TESTED: MONDAY SEP 19, 2016 - Raygan
 * WORKS: Valgrind reports no leaks. (as long as caller frees 'buf').
 * TODO: test buffer overflows
 * This first implementation just wraps the system calls to get effective uid username.
 */
int gridclient_get_username ( void **buf, size_t *len ) {
	struct passwd pwd;
	struct passwd *pwdp;
	size_t pwdrecsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	char *buf2;
	if( pwdrecsize == -1 )
		pwdrecsize = 1024; /*define our own max pwd recsize */
	buf2 = malloc(pwdrecsize);
	if( getpwuid_r( geteuid(), &pwd, buf2, pwdrecsize, &pwdp ) == -1 ) {
		perror("gridclient_get_username: ");
		return -1;
	}
	*len = strnlen(pwd.pw_name, LOGIN_NAME_MAX);
	*buf = strndup(pwd.pw_name, *len);

	free(buf2);
	return 0;
}

int gridclient_get_hostname ( void **buf, size_t *len ) {

	*buf = malloc(sizeof(char)*HOST_NAME_MAX);
	if( gethostname( *buf, HOST_NAME_MAX ) == -1 ) {
		fprintf(stderr, "gridclient_get_hostname: failed.\n");
		free(*buf);
		return -1;
	}
	char *bufs = *buf;		/* make compiler happy on next line */
	bufs[HOST_NAME_MAX-1]='\0';	/* avoid "dereferencing void ptr" */
	*len = strnlen(bufs, LOGIN_NAME_MAX);

	return 0;
}


/**************************************************
 *        struct snafu_fh support functions       *
 **************************************************/

void snafu_fh_release ( struct cosmos_grid *g, struct snafu_fh *fh ) {
	if( fh == NULL ) return;

	close( fh->fd[0] );
	close( fh->fd[1] );

	void *err = gridmsg( g, "fh:snafu/%lu releasedir", fh->id );
	gridfree( err );

	gridfree( fh );
}

