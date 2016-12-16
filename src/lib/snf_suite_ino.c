#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h> /* ENOTDIR */

#include "gridclient.h"
#include "snf_suite_ino.h"

#include <regex.h>


/** @file snf_suite_ino.c
 *
 * This needs to be merged (and function names reprefixed) into gridclient.c
 */



int snf_stat_ino( struct cosmos_grid *g, snafu_ino_t ino, struct stat *sb ) {

	/*
	 *  Try to do this with a gridmsg.
	 */
	struct stat *gridsb = gridmsg( g, "griduser: stat %lu", ino );


	/* TMP: implements "hello_ll.c" behaviour */

	sb->st_ino = ino;
	sb->st_uid = 1000;
	sb->st_gid = 1000;
	switch( ino ) {
		case 1:
			sb->st_mode = S_IFDIR | 0755;
			sb->st_nlink = 2;
			sb->st_size = 1;
			break;
		case 2:
			sb->st_mode = S_IFREG | 0444;
			sb->st_nlink = 1;
			sb->st_size = strlen( snafu_str );
			break;
		default:
			return -1;
	}
	return 0;
}

/*
 * Query grid database for `ino`.  If found, load it's access frame
 * and check the access path branches for a cached lookup,
 * check the dcel's directory for matches 
 */
snafu_ino_t snf_lookup_ino ( struct cosmos_grid *g, snafu_ino_t parent,
		             const char *name )
{
	struct buflink *rbuf = gridmsg( g, "griddb:///axpa lookup %lu %s", parent, name);
	snafu_ino_t ino = (snafu_ino_t)rbuf->c[0];

	/* RETURN INO VALUE */
	return ino;
}

struct snafu_fh *snf_opendir_ino( snafu_ino_t ino ) {
	/* need a file handle object */
	struct snafu_fh *fh = malloc(sizeof(struct snafu_fh));
	if( fh == NULL ) return NULL;
	memset(fh, 0, sizeof(*fh));


	/* prelim WORKING
	   test for ino here and create an in memory file 
	   with interleaved dirents and stats.  Put open FD in 'fi'.
	 */
	switch (ino) {
	case 1:
		/* dir:
		   	- "."
			- ".."
			- snafu_name
		 */

		if( pipe( fh->fd ) == -1 ) {
			free( fh );
			return NULL;
		}

		printf("snf_opendir_ino: %d, %d\n",fh->fd[0],fh->fd[1]);

		/* directory data */

			/* dirents */
		struct dirent dot = {
			.d_ino = 1,
			.d_name = ".",
		};
		struct dirent dotdot = {
			.d_ino = 1,
			.d_name = "..",
		};
		struct dirent file1 = {
			.d_ino = 2,
			.d_name = SNAFU_FILENAME,
		};
			
			/* stat info */
		struct stat dotstat = {
			.st_ino = 1,
			.st_mode = S_IFDIR | 0755,
			.st_nlink = 2,
			.st_uid = 1000,
			.st_gid = 1000,
		};
		struct stat dotdotstat = {
			.st_ino = 1,
			.st_mode = S_IFDIR | 0755,
			.st_nlink = 2,
			.st_uid = 1000,
			.st_gid = 1000,
		};
		struct stat file1stat = {
			.st_ino = 2,
			.st_mode = S_IFREG | 0444,
			.st_nlink = 1,
			.st_size = strlen(SNAFU_FILECONT),
			.st_uid = 1000,
			.st_gid = 1000,
		};

		/* serialize:
		   - dot
		   - dotstat
		   - dotdot
		   - dotdotstat
		   - file1
		   - file1stat
		 */


		char ctlcont[1] = { '1' };
		char ctlend[1] = { EOF };

#define WRITED(de) write( fh->fd[1], de, sizeof(struct dirent) )
#define WRITES(de) write( fh->fd[1], de, sizeof(struct stat) )
#define WRITECONT  write( fh->fd[1], ctlcont, sizeof(char) )
#define WRITEEND   write( fh->fd[1], ctlend, sizeof(char) )
		WRITED( &dot );
		WRITES( &dotstat );
		WRITECONT;
		WRITED( &dotdot );
		WRITES( &dotdotstat );
		WRITECONT;
		WRITED( &file1 );
		WRITES( &file1stat );
		WRITEEND;
		fh->empty = 0;	/* 0 == not empty */
		fdatasync(fh->fd[1]);
#undef WRITED
#undef WRITES
#undef WRITECONT
#undef WRITEEND
		return fh;
	case 2:
		fh->errornum = ENOTDIR;
		return fh;
	default:
		fh->errornum = ENOTDIR;
		return fh;
	}
}


struct snafu_fh *snf_open_ino ( snafu_ino_t ino ) {
	/* WORKING TEST
	   use a pipe
	 */
	struct snafu_fh *fh = malloc(sizeof(struct snafu_fh));
	memset(fh, 0, sizeof(*fh));

	if( pipe( fh->fd ) == -1 ) {
		free(fh);
		fprintf(stderr, "snf_open_ino: can't open pipe.\n");
		return NULL;
	}

	write( fh->fd[1], snafu_str, sizeof(char)*strlen(snafu_str)+1);
	fdatasync(fh->fd[1]);
	fh->size = sizeof(char)*strlen(snafu_str)+1;
	return fh;
}


/* candidates to go in `snf_suite_path.c` */
snafu_ino_t snf_geturl( struct cosmos_grid *g, const char *url ) {
	if( g == NULL ) return 0;
	if( url == NULL ) return 0;

	return 0;
}

struct uri_parser_state {
	char *base;
	size_t len;
	char *pos;
	size_t rem;
};
static void *snf_init_uri_parser() {
	struct uri_parser_state *ps = malloc(sizeof(*ps));
	if( ps == NULL ) return NULL;
	memset(ps, 0, sizeof(*ps));
	return (void *) ps;
}
static char *snf_get_uri_base( void *uri_parser, const char *uri ) {
	if( uri_parser == NULL ) return NULL;
	if( uri == NULL ) return NULL;

	char *erepat = "^snafu://[^/]+";

	int status;
	regex_t re;
	size_t pn = 1;
	regmatch_t pm[1];

	if( regcomp(&re, erepat, REG_EXTENDED ) != 0 ) {
		fprintf(stderr, "snf_get_uri_base: trouble compiling RE.\n");
		return NULL;
	}
	status = regexec( &re, uri, pn, pm, 0 );
	regfree( &re );
	if( status != 0 ) {
		fprintf(stderr, "snf_get_uri_base: regexec err or no match.\n");
		return NULL;
	}

	size_t matchlen = pm[0].rm_eo - pm[0].rm_so;

	struct uri_parser_state *ps = (struct uri_parser_state *) uri_parser;

	ps->base = (char *)uri;
	ps->len = strlen(uri);

	ps->pos  = (char *)uri + pm[0].rm_eo;
	ps->rem = ps->len - matchlen;

	char *res = strndup(uri, matchlen);


	return res;
}

/*
 * returns an `ino` corresponding to `str`.
 *
 * if no entry exists, create an entry for `str` with a new `ino`.
 */
static snafu_ino_t snf_get_root_ino( struct cosmos_grid *g, const char *str ) {
	if( g == NULL ) return 0;
	if( str == NULL ) return 0;

	snafu_ino_t ino = 0;

	ino = (snafu_ino_t) gridmsg( g, "griddb:///rootstr get %s", str );
	if( ino == 0 ) {
		ino = (snafu_ino_t) gridmsg( g, "griddb:///axpa new" );
		if( ino == 0 )
			return 0;

		gridmsg( g, "griddb:///rootstr set %s %lu", str, ino );
	}
	return ino;
}

static int snf_get_next_uri_segment( void *uri_parser, char **str ) {
	// struct uri_parser_state *ps = (struct uri_parser_state *) uri_parser;
	return -1;
}

static void snf_cleanup_uri_parser( void *uri_parser ) {
	struct uri_parser_state *ps = (struct uri_parser_state *) uri_parser;
	free( ps );
}

snafu_ino_t snf_lookup_uri ( struct cosmos_grid *g, const char *uri ) {
	if( g == NULL ) return 0;
	if( uri == NULL ) return 0;

	char *str;
	void *uri_parser;
	snafu_ino_t ino;

	uri_parser = snf_init_uri_parser();
	if( uri_parser == NULL ) return 0;

	str = snf_get_uri_base( uri_parser, uri );
	if( str == NULL ) return 0;

	ino = snf_get_root_ino( g, str );
	if( ino == 0 ) return 0;

	while ( snf_get_next_uri_segment( uri_parser, &str ) == 0 ) {
		ino = snf_lookup_ino( g, ino, str );
	}

	snf_cleanup_uri_parser( uri_parser );

	return ino;
}

snafu_ino_t snf_bind_mountpoint_uri( struct cosmos_grid *g, const char *uri ) {


	/* preserve access to underlying file system */
	/* 1) find dcel which references the localhost's mountpoint. */
	snafu_ino_t ino = snf_lookup_uri( g, uri );

	/* 2) initialize preservation server module with dcel's source addr. */

	/* 3) replace the dcel's server module with preservation server mod. */
	/* 4) save the dcel's previous server module for restore */

	/* TEMPORARY RETURN VALUE */
	return ino;

	/* DEFAULT RETURN VALUE */
	return 0;
}

void snf_release_mountpoint_ino( struct cosmos_grid *g, snafu_ino_t ino ) {
	/* restore normal access to underlying file system */
	/* 1) find dcel which uses the preservation server
	   2) call the server's release function
	   3) if retaincount is 0, replace the preservation server module
	   	with the dcel's previous server module.
	 */
}
