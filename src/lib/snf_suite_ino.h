#ifndef _SNF_SUITE_INO_H_
#define _SNF_SUITE_INO_H_

#include "cosmos_grid.h"



int snf_stat_ino( struct cosmos_grid *g, snafu_ino_t ino, struct stat *sb );
snafu_ino_t snf_lookup_ino( struct cosmos_grid *g, snafu_ino_t ino,
		            const char *name );
struct snafu_fh *snf_opendir_ino( snafu_ino_t ino );
void snf_releasedir_fh ( struct snafu_fh *fh );

struct snafu_fh *snf_open_ino ( snafu_ino_t ino );
void snf_release_fh ( struct snafu_fh *fh );


/* candidates to go in snf_suite_path.h */
/*					*/
/*					************************************/

/**
 * Helper function creates 'snafu' scheme URI from filepath.
 * The resulting char string should be free'd by the caller.
 */
char *snf_filepath_to_uri( const char *filepath );

/**
 * Binds the localhost filetree contents of mountpoint onto the mountpoint uri
 * using a special server module to preserve the localhost contents so that 
 * a FUSE volume can be mounted over the filetree.
 */
snafu_ino_t snf_bind_mountpoint_uri( struct cosmos_grid *g, const char *uri );

/**
 * Releases the bind mount on `mountpoint`.  This version takes an `ino`
 * and would be part of `snf_suite_ino.h`.
 *
 * I might not implement a `snf_release_mountpoint_uri()` as you can
 * a) use `snf_lookup_uri()` to find the ino to release or
 * b) keep track of your FUSE volume's root `ino` to release.
 */
void snf_release_mountpoint_ino( struct cosmos_grid *g, snafu_ino_t ino );

/**
 * resolve URL by pathwalk, build Cosmos cascade, run lookup module.
 */
snafu_ino_t snf_lookup_uri( struct cosmos_grid *g, const char *uri );

#endif /*!_SNF_SUITE_INO_H_*/
