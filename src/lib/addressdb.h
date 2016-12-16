#ifndef _LIBGRID_ADDRESSDB_H_
#define _LIBGRID_ADDRESSDB_H_

/**
 * @file addressdb.h
 *
 * Client Interface for to Address Database.  Limits actions to functions needed to be performed by the client.  Some accessor "set" functions are not available here because the "set" function is performed behind the curtain and may be the result of a complex calculation.
 *
 * Used by libgridhost.c and server_module.c
 *
 *
 * 
 */

/**
 * A grid address record.
 *
 */
struct grid_addr {
	char  *url;
	size_t urllen;
	int    scheme;
	void  *user;		/* possibly a complex user object */
	void  *host;		/* possibly a complex host object */
	char  *filepath;	
	size_t filepathlen;
	char  *fragstr;
	size_t fragstrlen;
	void  *fcel;		/* a pointer to the fragment cel */
	void  *server_module;	/* possibly complex */
	void  *server_addrno;	/* identifies the fragment in server space */
	void  *server_addrno_par;	/* identifies the parent in server space */
	void  *bundleid;
	void  *grid_addrno;
};

/**
 * add addr1 to the database.
 *
 * if addr2 is not NULL, then bind addr1 with addr2's bundle.
 *
 *
 * @param grid  must be a handle returned by grid_open() 
 * @param size1 the size of addr1 in bytes
 * @param size2 the size of addr2 in bytes (can be 0)
 * @param addr1 should be a NULL-terminated string
 * @param addr2	can be NULL or a NULL-terminated string
 *
 * @param res	the resulting grid address record

 * @return an ino which URL binds with
 *
 * The return type should accomodate fuse_ino_t
 */
int gridaddr_addurl( void *grid, size_t size1, size_t size2, char *url1, char *url2, struct grid_addr *res );


/**
 * remove an address from the database
 *
 * @param grid
 * @param addr1
 * @return an ino if the URL was bundled, 0 if alone
 */
unsigned long int gridaddr_rmurl( void *grid, char *url1 );


/* ADDR RECORD ACCESSOR FUNCTIONS
   available to users
   other maniupulations may be defined elsewhere. */

void *gridaddr_get_server( struct grid_addr *ga );
void *gridaddr_get_addrno( struct grid_addr *ga );
void *gridaddr_get_bundleno( struct grid_addr *ga );



#endif /*!_LIBGRID_ADDRESSDB_H_*/
