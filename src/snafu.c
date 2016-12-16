/*
 * Snafu File System
 * Copyright (C) 2016  Raygan Henley <raygan@raygan.com>
 *
 * Valgrind detects a memory leak from:
 *	 libfuse/fuse_opt.c/add_opt_common()/realloc()
 *
 */

/**
 * @file
 *
 * Snafu FUSE Volume
 *
 * Uses a minimal set of calls to `gridclient`.
 *
 * The workhorse API function is `gridmsg()` which acts like `printf()`
 * but outputs to the grid socket.
 *
 * gridmsg returns a pointer to memory which is sized according to
 * the return type of the `gridmsg` protocol function.
 *
 * There are a few objects
 * - struct cosmos_grid
 * - struct snafu_fh
 * - snafu_ino_t
 *
 */

/*
  DERIVED FROM THIS:

  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello_ll.c `pkg-config fuse --cflags --libs` -o hello_ll
*/

#define FUSE_USE_VERSION 26

#include <fuse_lowlevel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>


#include "lib/gridclient.h"

/**
 * A snafu volume structure.
 */
struct snafu_fs {
	struct cosmos_grid *grid;
	snafu_ino_t rootino;
};


/**
 * Initialize the volume structure.
 *
 * Responsibilities:
 * - Preserve access to the mountpoint before FUSE mount.
 * - Setup Grid
 */
static struct snafu_fs *snafu_init ( const char *mountpoint ) {
	if( mountpoint == NULL ) return NULL;

	char *errstr = NULL;

	struct snafu_fs * fs = malloc(sizeof(*fs));
	if( fs == NULL ) {
		errstr = "failed to allocate `fs`";
		goto abort_null;
	}
	memset(fs, 0, sizeof(*fs));

	struct cosmos_grid *grid = grid_open();
	if( grid == NULL ) {
		errstr = "failed to open grid";
		goto abort_grid;
	}

	/**
	 * Convert mountpoint's filepath to a file scheme URL.
	 * ie "file://<user>@<host>/<absolutepath>"
	 */
	char *url = grid_filepath_to_uri( mountpoint, "file" );
	if( url == NULL )
		goto abort_uri;


	void *err = gridmsg( grid, "%s server:preserve", url );
	if( err == NULL ) {
		errmsg = "failed to preserve mountpoint";
		goto abort_gridmsg;
	}
	gridfree( err );

	char *snafurl = grid_filepath_to_uri( mountpoint, "snafu" );
	if( url == NULL )
		goto abort_uri;

	snafu_ino_t *rootino;
	rootino = gridmsg( grid, "%s bind %s", snafurl, url );
	if( rootino == 0 ) {
		goto abort_gridmsg;
	}
	free( url );

	fs->rootino = *rootino;
	gridfree( rootino );

	fs->grid = grid;
	return fs;
abort_gridmsg:
	free( snafurl );
	free( url );
abort_uri:
	grid_close( grid );
abort_grid:
	free(fs);
abort_null:
	fprintf(stderr, "snafu_init: %s\n", errstr);
	return NULL;
};

/**
 * Cleanup the volume structure.
 */
static void snafu_cleanup ( struct snafu_fs *fs ) {
	if( fs == NULL ) return;
	if( fs->grid != NULL ) {
		void *err = gridmsg( fs->grid,
			"ino:snafvol/%lu server:unpreserve", fs->rootino );
		gridfree( err );
		grid_close( fs->grid );
	}
	free( fs );
};

/**
 * This is close to the reference implementation
 * provided by FUSE example 'hello_ll.c'.
 *
 * Observed (via debug mode) to be first call after "access()".
 */
static void snafu_getattr ( fuse_req_t req, snafu_ino_t ino,
				struct fuse_file_info *fi ) {

	fprintf(stderr, "snafu_getattr: ino %lu\n", ino);

	struct snafu_fs *fs = fuse_req_userdata( req );
	if( fs == NULL ) {
		fprintf(stderr, "fs == NULL\n");
		fuse_reply_err( req, ENOENT );
		return;
	}
		

	struct cosmos_grid *g = fs->grid;
	if( g == NULL ) {
		fprintf(stderr, "snafu_getattr: grid is NULL\n");
		fuse_reply_err( req, ENOENT );
		return;
	}

	/* --- based on reference implementation from hello_ll.c --- */
	struct stat *stbuf;
	if( (stbuf = gridmsg(fs->grid, "ino:snafvol/%lu stat", ino)) == NULL )
	{
		fuse_reply_err( req, ENOENT );
	} else {
		fuse_reply_attr( req, stbuf, 1.0 );
		gridfree( stbuf );
	}
}

/**
 * 'lookup()' uses 'fuse_reply_entry()' to pass back
 * a 'fuse_entry_param' which has the interesting members
 * 'ino', 'generation' and 'attr'.
 *
 * We call 'snf_stat_ino()' to assign 'attr'.
 * We assign 'ino' the value of 'attr.st_ino'.
 *
 * The actual lookup is performed by the snafu backend
 * by calling snf_lookup_ino()
 *
 * I'm not sure how 'generation' factors into snafu yet.  TBD.
 * (We'll follow the reference implementation of hello_ll.c
 * and totally ignore it like a bastard child...for now.)
 */
static void snafu_lookup (fuse_req_t req, snafu_ino_t parent, const char *name)
{
	fprintf(stderr, "snafu_lookup\n");

	struct snafu_fs    *fs = fuse_req_userdata( req );
	if( fs == NULL )
		goto abort;

	struct cosmos_grid *g  = fs->grid;
	if( g == NULL )
		goto abort;

	snafu_ino_t *ino;
	ino = gridmsg( g, "ino:snafvol/%lu lookup %s", parent, name );
	if( *ino == 0 )
		goto abort;

	struct fuse_entry_param e;
	memset(&e, 0, sizeof(struct fuse_entry_param));

	struct stat *sb;
	if( (sb = gridmsg( g, "ino:snafvol/%lu stat", *ino ) == -1 )
		goto abort_stat;
	gridfree( ino );

	memcpy( &e.attr, sb, sizeof(struct stat));
	gridfree( sb );

	e.ino = e.attr.st_ino;
	e.attr_timeout = 1.0;
	e.entry_timeout = 1.0;

	fuse_reply_entry(req, &e);

	return;
abort_stat:
	gridfree( ino );
abort:
	/* TODO: improve snafu error messages */
	fuse_reply_err(req, ENOENT);
	return;
}

struct dirbuf {
	char *p;
	size_t size;
};
#define min(x, y) ((x) < (y) ? (x) : (y))

static int reply_buf_limited(fuse_req_t req, const char *buf, size_t bufsize,
			     off_t off, size_t maxsize)
{
	if (off < bufsize)
		return fuse_reply_buf(req, buf + off,
				      min(bufsize - off, maxsize));
	else
		return fuse_reply_buf(req, NULL, 0);
}

/**
 * Directory support provided by the FUSE hello_ll.c example.
 */
static void dirbuf_add ( fuse_req_t req, struct dirbuf *b, const char *name,
			 struct stat *stbuf) {
	size_t oldsize = b->size;
	b->size += fuse_add_direntry( req, NULL, 0, name, NULL, 0);
	b->p     = (char *) realloc(b->p, b->size);
	fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, stbuf,
			  b->size);
}

/**
 * snauf_opendir() must have passed an open file descriptor in 'fi'.
 * We read from 'fi': `struct dirent`s interleaved with `struct stat`s.
 * 
 * use `fuse_add_direntry()` to add `d_name` and `struct stat` to a buffer.
 *
 * use `fuse_reply_buf()` to return the buffer to FUSE.
 * 
 * - dirent as defined in POSIX: d_ino, d_name[NAME_MAX]
 * - stat as defined in POSIX.
 */
static void snafu_readdir ( fuse_req_t req, snafu_ino_t ino, size_t size,
			    off_t off, struct fuse_file_info *fi ) {

	struct snafu_fs *fs = fuse_req_userdata( req );
	if( fs == NULL ) {
		fuse_reply_err(req, ENOTDIR);
		return;
	}

	struct snafu_fh *fhp = (struct snafu_fh *)fi->fh;
	if( fhp == NULL ) {
		fuse_reply_err(req, ENOTDIR);
		return;
	}

	
	if( ino != 1 )
		fuse_reply_err(req, ENOTDIR);
	else {
		if( fhp->empty == 1 ) {
			fuse_reply_buf(req, NULL, 0);
			return;
		}

		struct dirbuf b;
		memset(&b, 0, sizeof(b));

		struct dirent e;
		memset(&e, 0, sizeof(e));

		struct stat sb;
		memset(&sb, 0, sizeof(sb));

		char ctl = '\0';

readloop:
		ctl = '\0';
		if( read( fhp->fd[0], &e, sizeof(struct dirent) ) == -1 )
		{
			perror(NULL);
			fprintf(stderr, "trouble reading dirent from pipe\n");
			goto breakloop;
		}
		if( read( fhp->fd[0], &sb, sizeof(struct stat) ) == -1 )
		{
			perror(NULL);
			fprintf(stderr, "trouble reading stat from pipe\n");
			goto breakloop;
		}

		dirbuf_add( req, &b, e.d_name, &sb );

		if( read( fhp->fd[0], &ctl, sizeof(char) ) == -1 )
		{
			perror(NULL);
			fprintf(stderr, "trouble reading control from pipe\n");
			goto breakloop;
		}
		if( ctl == '1' ) {
			goto readloop;
		}
breakloop:
		fhp->empty = 1;
		reply_buf_limited(req, b.p, b.size, off, size);
		free(b.p);
	}
}


/**
 * DRAFT:
 * First try to implement a directory using a pipe.
 *
 * 'fi->fh->fd[0]' references the pipe's file descriptor for read.
 * 'fi->fh->fd[1]' references the pipe's file descriptor for write.
 *
 * uses `snf_suite` to open a fd to a snafu directory stream.
 *
 */
static void snafu_opendir ( fuse_req_t req, fuse_ino_t ino,
		            struct fuse_file_info *fi ) {
	printf("snafu_opendir:\n");
	struct snafu_fs *fs = fuse_req_userdata( req );
	if( fs == NULL ) {
		fuse_reply_err( req, EACCES );
		return;
	}

	struct cosmos_grid *g = fs->grid;
	if( g == NULL ) {
		fuse_reply_err( req, EACCES );
		return;
	}

	struct snafu_fh *fh = gridmsg( g, "ino:snafvol/%lu opendir", ino );
	if( fh == NULL ) {
		fprintf(stderr, "snf_opendir_ino() returned NULL\n");
		fuse_reply_err( req, EACCES );
		return;

	} else if( fh->errornum != 0 ) {
		fprintf(stderr, "snf_opendir_ino() returned error\n");
		fuse_reply_err( req, fh->errornum );
		return;
	}
	
	
	fi->fh = (uint64_t)fh;
	printf("fi->fh: %lu	open\n", fi->fh);

	fuse_reply_open( req, fi );
	return;
}

/**
 * closes pipes on `fi->fh->fd[]`
 * free's `fi->fh`
 */
static void snafu_releasedir ( fuse_req_t req, fuse_ino_t ino,
		               struct fuse_file_info *fi ) {
	printf("snafu_releasedir:\n");
	
	struct snafu_fs *fs = fuse_req_userdata( req );
	if( fs == NULL ) {
		return;
	}

	struct cosmos_grid *g = fs->grid;
	if( g == NULL ) {
		return;
	}

	struct snafu_fh *fh = (struct snafu_fh *)fi->fh;

	snafu_fh_release( g, fh ); 

	fprintf(stderr, "fi->fh: %ld close\n", fi->fh);
	fuse_reply_err( req, 0 );
}

/**
 * This opens a snafu file handle for reading and writing from Gridclient Lib.
 */
static void snafu_open ( fuse_req_t req, snafu_ino_t ino,
		         struct fuse_file_info *fi )
{
	if (ino != 2)
		fuse_reply_err(req, EISDIR);
	else if ((fi->flags & 3) != O_RDONLY)
		fuse_reply_err(req, EACCES);
	else {
		struct snafu_fh *fh = gridmsg( "ino:snafvol/%lu open",ino );
		fi->fh = (uint64_t) fh;
		fuse_reply_open(req, fi);
	}
}

/**
 * Read from pipe on fi->fh->fd[0]
 */
static void snafu_read ( fuse_req_t req, snafu_ino_t ino, size_t size,
		         off_t off, struct fuse_file_info *fi )
{
	char buf[256];

	struct snafu_fh *fhp = (struct snafu_fh *)fi->fh;
	if( fhp == NULL ) {
		fprintf(stderr, "error: filehandle is NULL\n");
	}

	size_t readsize = fhp->size;
	if( readsize > 256 ) readsize = 256;
	if( readsize > size ) readsize = size;

	size_t bytesread = read( fhp->fd[0], &buf, readsize );

	reply_buf_limited(req, buf, bytesread, off, size);
}
/**
 * FUSE wrapper to release file
 */
static void snafu_release ( fuse_req_t req, snafu_ino_t ino,
		      struct fuse_file_info *fi )
{
	struct snafu_fh *fh = (struct snafu_fh *) fi->fh;

	snafu_fh_release( g, fh );

	fuse_reply_err( req, 0 );
}

static struct fuse_lowlevel_ops hello_ll_oper = {
	.lookup		= snafu_lookup,
	.getattr	= snafu_getattr,
	.opendir	= snafu_opendir,
	.readdir	= snafu_readdir,
	.releasedir	= snafu_releasedir,
	.open		= snafu_open,
	.read		= snafu_read,
	.release	= snafu_release,
};


/**
 * `snafu_init()` must go before `fuse_mount()` in order
 * to preserve the contents of the underlying mountpoint.
 */
int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct fuse_chan *ch;
	char *mountpoint;
	int err = -1;

	if(fuse_parse_cmdline(&args, &mountpoint, NULL, NULL) != -1) {

		struct snafu_fs *fs = snafu_init(mountpoint);
		if( fs == NULL ) {
			fprintf(stderr, "main: failed to init file system.\n");
			goto abort;
		}

		if((ch = fuse_mount(mountpoint, &args)) != NULL) {
		struct fuse_session *se;

		se = fuse_lowlevel_new(&args, &hello_ll_oper,
				       sizeof(hello_ll_oper), fs);
		if (se != NULL) {
			if (fuse_set_signal_handlers(se) != -1) {
				fuse_session_add_chan(se, ch);
				err = fuse_session_loop(se);
				fuse_remove_signal_handlers(se);
				fuse_session_remove_chan(ch);
			}
			fuse_session_destroy(se);
		}
		fuse_unmount(mountpoint, ch);
	}
	snafu_cleanup( fs );
	fuse_opt_free_args(&args);
	}

	return err ? 1 : 0;
abort:
	fprintf(stderr, "snafu::main() aborted.\n");
	return 1;
}
