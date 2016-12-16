#ifndef _GRIDCLIENT_H_
#define _GRIDCLIENT_H_

/** @file
 *
 * The gridclient library provides access to the grid.
 *
 * The grid is accessed over a socket to a 'griduser' process.
 * There should be only one griduser process active per useraccount
 * on a given machine.
 *
 * The primary purpose is to be a front end for grid messages.
 * A grid message send and retrieve arbitrary data types.
 * The gridclient library tries to avoid knowledge with the user's datatypes.
 * Instead focuses on getting this data in and out of the grid message.
 *
 * Prefix: "grid_"
 *
 */

#include "cosmos_grid.h"
#include "snafu_fh.h"
#include "snafu_ino.h"

/**
 * redefine this instead of including fuse headers.
 * dev only
 */
typedef unsigned long fuse_ino_t;


/**
 * temporary file system contents - dev only.
 */
#define SNAFU_FILENAME "snafu.tmp"
#define SNAFU_FILECONT "Snafu Word Up!\n"
static const char *snafu_str = SNAFU_FILECONT;


/**
 * Returns a handle to a grid object.
 *
 * The handle contains a socket connection to a `griduser` daemon.
 * If there is no running `griduser` daemon owned by the current user
 * then libgridclient executes one.
 *
 * <WORKING...>
 * For convenience, grid_open() also runs Cosmos to setup
 * the User's Start Access Context and saves this in the grid object.
 */
struct cosmos_grid *grid_open( void );

/**
 * Close handle to grid object.
 */
void grid_close( struct cosmos_grid *grid );


/**
 * @return username string
 */
int gridclient_get_username( void **buf, size_t *len );

/**
 * @return hostname string
 *
 * This is more or less useful.  But a host, can go by so many names.
 * This can be used for rudemetary host identification.
 * But more thorough identification, may require advanced objects.
 */
int gridclient_get_hostname( void **buf, size_t *len );

char *grid_filepath_to_uri ( const char *filepath, char *scheme ) {

void snafu_fh_release( struct cosmos_grid *g, struct snafu_fh *fh );

#endif /*!_GRIDCLIENT_H_*/
