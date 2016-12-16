Gridclient Library {#gridclient}
==================

Gridclient Library provides high-level access to the grid.

`gridclient.h`


Like FUSE there is a high-level pathname based API and a low level ino based API.

	ino = grid_lookup_uri( "snafu://bangfish.local/home/redboy/theproject" )
	==
	ino = grid_lookup_ino( 0, "snafu://bangfish.local/" )
	ino = grid_lookup_ino( ino, "home" )
	ino = grid_lookup_ino( ino, "redboy" )
	ino = grid_lookup_ino( ino, "theproject" )




\section gridclient_api		Gridclient API


Grid handle:

	struct gridhandle *grid_open( void );

	void
	grid_close( struct gridhandle * );


URI formatter:

	char *
	grid_filepath_to_uri( char *mountpoint, char *uri_scheme_name );


Overlay filesystem helper: (use before FUSE mounts with non-empty option)

	void *
	grid_server_preserve_uri( struct gridhandle *, char *uri );

	void *
	grid_server_unpreserve_ino( struct gridhandle *, snafu_ino_t );

	snafu_ino_t
	grid_bind_uris( struct gridhandle *, char *uri1, char *uri2 );


File System Ops:

	struct stat *
	grid_stat_ino( struct gridhandle *, snafu_ino_t );

	snafu_ino_t
	grid_lookup_ino( struct gridhandle *, snafu_ino_t parent, char *name );

	struct snafu_fh *
	grid_open_ino( struct gridhandle *, snafu_ino_t );
	
	struct snafu_fh *
	grid_opendir_ino( struct gridhandle *, snafu_ino_t );

	void *
	grid_release_fh( struct gridhandle *, struct snafu_fh * );






