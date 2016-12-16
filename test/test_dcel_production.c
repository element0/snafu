/**
 * @file
 *
 * testing dcel production functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lib/dcel.h"
#include "../src/lib/dcel.c"

#include "../src/lib/config.h"
#include "../src/lib/config.c"

#include "../src/lib/aframe.h"
#include "../src/lib/aframe.c"


/** a temporary definition of TIMEOUT */
#define TIMEOUT 0


int main( int argc, char *argv[] ) {
	if(argc != 2) {
		printf("usage: CMD PATH\n");
		return -1;
	}

	/* config */
	struct cosmos_program_config *conf = cosmos_program_defaults();
	char *cosmosroot = getenv( "COSMOSROOT" );
	if( cosmosroot == NULL && (cosmosroot = conf->cosmosroot) == NULL ) {
		printf("You should set COSMOSROOT in the env.\n");
		return -1;
	}

	/* ROOT ACCESS FRAME */


	/* protodcel */
	struct hiena_dcel *protodcel = new_dcel_from_filepath( cosmosroot );
	if( protodcel == NULL ) {
		fprintf(stderr, "protodcel was NULL.\n");
		return -1;
	}


	/* root dcel */
	struct hiena_dcel *dcel = new_dcel_from_filepath( argv[1] );
	if( dcel == NULL ) {
		fprintf(stderr, "root dcel was NULL.\n");
		dcel_cleanup( protodcel );
		return -1;
	}
	
	struct cosmos_aframe *roota = aframe_new();
	roota->dcel   = dcel;
	roota->parent = NULL;
	roota->cosm   = overloadcosm( loadcosm( protodcel ), loadcosm( dcel ));

	while( dcel = update_dcel_gridsources( dcel, roota->cosm )
			!= dcel && !TIMEOUT ) {
		roota->cosm = overloadcosm( roota->cosm, loadcosm( dcel ));
	}

	dcel_cleanup( protodcel );
	dcel_cleanup( dcel );

	/* DESCENDANT ACCESS FRAMES */
	dcel = new_dcel_from_uri( argv[1] );

	dcel = update_dcel_gridsources( dcel, NULL );
	dcel_cleanup( dcel );
	
	return 0;
}
