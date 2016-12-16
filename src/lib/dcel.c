#include "dcel.h"

struct hiena_dcel *new_dcel() {
	struct hiena_dcel *dcel = malloc(sizeof(*dcel));
	if(dcel != NULL)
		memset(dcel, 0, sizeof(*dcel));
	return dcel;
}


void prod_cleanup( struct hiena_prod *prod ) {
	if( prod == NULL ) return;

	if( prod->server != NULL )
		free( prod->server );
	if( prod->addr   != NULL )
		free( prod->addr );

	free( prod );
}


void frag_cleanup( struct hiena_frag *frag ) {
	if( frag == NULL ) return;
	prod_cleanup( frag->prod );
	free( frag );
}


void dcel_cleanup( struct hiena_dcel *dcel ) {
	if( dcel == NULL ) return;
	frag_cleanup( dcel->frag );
	free(dcel);
}

struct hiena_frag *new_frag() {
	struct hiena_frag *frag = malloc(sizeof(*frag));
	if(frag != NULL)
		memset(frag, 0, sizeof(*frag));
	return frag;
}


struct hiena_prod *new_prod() {
	struct hiena_prod *prod = malloc(sizeof(*prod));
	if(prod != NULL)
		memset(prod, 0, sizeof(*prod));
	return prod;
}


struct hiena_prod *source_prod( char *server, char *addr ) {
	if( server == NULL || addr == NULL )
		return NULL;

	struct hiena_prod *prod = new_prod();
	if( prod == NULL ) return NULL;

	prod->server = strndup( server, strlen( server ) );
	prod->addr   = strndup( addr, strlen( addr ) );

	printf("source %s %s\n", server, addr );

	return prod;
}

struct hiena_dcel *new_dcel_from_uri( char *uri ) {
	if( uri == NULL ) return NULL;

	char *curp;
	size_t len = strlen( uri );
	size_t count = 0;

	for( curp = uri; curp <= uri+len && *curp != ':'; curp++, count++) {
	}
	char *scheme = NULL;
	if( curp[0] == ':' ) {  
		scheme = strndup( uri, count );
	}

	for( count = 0 ; curp <= uri+len && count != 3; curp++ ) {
		if( *curp == '/' ) count++;
	}
	char *path = NULL;
	if( count == 3 ) {
		curp--;
		len = strlen( curp );
		path = strndup( curp, len );
	}
	struct hiena_dcel *dcel = NULL;
	if( scheme != NULL && path != NULL ) {
		dcel = new_dcel();
		if( dcel == NULL ) goto abort;

		if(( dcel->frag = new_frag() ) == NULL ) goto abort;

		if(( dcel->frag->prod = source_prod( scheme, path )) == NULL )
			goto abort;

	}
	if( scheme != NULL ) free(scheme);
	if( path   != NULL ) free(path);
	return dcel;

abort:
	if( scheme != NULL ) free(scheme);
	if( path   != NULL ) free(path);
	dcel_cleanup(dcel);
	return NULL;
}

struct hiena_dcel *new_dcel_from_filepath( char *fpath ) {
	struct hiena_dcel *dcel = new_dcel();
	if( dcel == NULL ) return NULL;

	if(( dcel->frag = new_frag() ) == NULL ) goto abort;

	if(( dcel->frag->prod = source_prod( "file", fpath )) == NULL )
		goto abort;

	return dcel;
abort:
	dcel_cleanup(dcel);
	return NULL;
}

/**
 * FIRST DRAFT: This is a stub.  It does nothing.  The intent is that it will eventually refresh the Grid Topology and from that, will generate (or retrieve from cache) additional dcels, bind those cells together and return the bundle cell.
 */
struct hiena_dcel *update_dcel_gridsources( struct hiena_dcel *dcel,
						void *cosmptr ) {
	/* if this function fails, it returns `dcel` */
	if( dcel == NULL || cosmptr == NULL ) return dcel;

	struct hiena_dcel *cosm = cosmptr;

	return dcel;
}
