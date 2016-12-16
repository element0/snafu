#ifndef _LIBGRID_GRIDUSER_H_
#define _LIBGRID_GRIDUSER_H_

/**
 * @file
 *
 * This is the interface for `griduserhost`.
 *
 * Some of these functions are placeholders
 * for commands which will be invoked by "grid instructions"
 * which are read by the 'griduserd' from the 'gridusersock'.
 * It remains TBD how these commands will be implemented.
 *
 * Griduserd loads 'lookup module' and 'server modules'.
 *
 * Griduserd sends messages over `griddbsock`
 * to access `griddb`.
 *
 * Access path `aframe`, domain cel `dcel`, and others
 * are stored in `griddb`.
 */


/**
 * Keeps loaded modules.
 * - data source servers
 * - path lookup parser
 */
struct griduserhost {
	void *server;
	void *lookup;
};


/**
 * @param  access path 'aframe' to run the lookup inside.
 * @param  'name' input string for 'lookup module'
 * 
 * @return message { access path ID 'ino', error codes }
 *
 */
unsigned long int griduser_lookup( void *aframe, char *name );

#endif /*!_LIBGRID_GRIDUSER_H_*/
