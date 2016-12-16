#ifndef _COSMOS_AFRAME_H_
#define _COSMOS_AFRAME_H_

/** @file
 *
 * Domain cell API.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdint.h>


/**
 * A cosmos aframe performs the function of a path segment.
 *
 * "A-frame" stands for "access frame".
 *
 *
 * A cosmos-hiena file system has two kinds of inodes: dcels and cosmos_aframe.
 * The dcel records the data structure of boundeded stream of data.
 * The aframe records context information used to access the dcel from
 * a given access path.  Different paths to the same location have different
 * contexts.  The dcel may contain map cells created by one aframe but
 * not another.  Map cels may have user restrictions.  But even if a map cell
 * has been produced from one context, it may be fair for the other context
 * to access it.  But this doesn't mean the other context inherits any of the
 * other.
 *
 * The aframe structure functions as the primary inode exposed to the
 * FUSE interface.  The dcel is handled internally.
 *
 */
struct cosmos_aframe {
	struct stat sbuf;
	int error;
	void *cosm;		/* a Dcel structure */	
	void *env;		/* a vector of strings */
	void *dcel;		/* a Dcel structure */
	void *lookup_cache;	/* key,value { lookup_str_id, aframe_id } */
	struct cosmos_aframe *parent;
};

struct cosmos_aframe *aframe_new();

/**
 * Initialize an aframe from a .cosm file pattern.
 *
 */
struct cosmos_aframe *cosmos_new_aframe_from_gridaddr();

/**
 * Typecase an aframe from a buffer pointer.
 * supposedly this buffer contains an aframe pattern
 * test the buffer for aframe.
 * use the buffer address don't duplicate the buffer.
 * 
 * Aframe will be required to dispose of buffer.
 */
int aframe_at_buffer( struct cosmos_aframe **a,
		void *buf, size_t len );

int aframe_get_error( struct cosmos_aframe *aframe );


/**
 * Initialize a Start Access Context from a system library installation.
 *
 */
struct cosmos_aframe *cosmos_init_aframe();


/**
 * Cleanup the aframe.
 *
 * Also required to cleanup any buffer acquired through 'aframe_from_buffer'
 */
void aframe_cleanup( struct cosmos_aframe * );


/**
 * Get stat buffer for aframe
 *
 * This follows the format for stat (2).
 */
int aframe_stat( struct cosmos_aframe *aframe, struct stat *buf );


/**
 * Re-entrant read directory function.
 *
 * Follows the format of readdir_r.
 * Returns a conventional dirent.
 * Proof of concept only uses d_ino and d_name per POSIX.
 *
 * This version does not rely on calling aframe_opendir but
 * instead uses struc cosmos_aframe as a DIR object.
 */
int aframe_readdir_r( struct cosmos_aframe *aframe, struct dirent *entry, struct dirent **result );

/**
 * Open a file stream on a aframe.
 *
 * Proof of concept uses return type void*.
 *
 * 'mode' follows FUSE requirement rather than fopen (3).
 */
uint64_t aframe_fopen( struct cosmos_aframe *aframe, int mode );

/**
 * Read a file stream opened from a aframe.
 *
 * Proof of concept uses file handle 'fh' type uint64_t as required by FUSE.
 *
 */
size_t aframe_fread( char *buf, size_t size, uint64_t fh );

/**
 * Close file stream handle.
 *
 * Proof of concept uses uint64_t as required by FUSE.
 *
 */
int aframe_fclose( uint64_t fh );

#endif /*!_COSMOS_AFRAME_H_*/
