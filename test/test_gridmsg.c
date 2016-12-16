#include <stdio.h>
#include "../src/lib/gridclient.h"
#include "../src/lib/gridmsg.h"

int main( int argc, char *argv[] ) {

	struct cosmos_grid *g = grid_open();

/*
	struct gridmsg *msg = gridmsg_new( g );
	gridmsg_print( msg );
	printf("\n");
	gridmsg_cleanup( msg );
*/

	struct buflink *b = gridmsg(g, "griddb://localhost/rootstrdb set %s %lu", "bangfish.local", 12 );

	if( b == NULL ) {
		fprintf(stderr, "test_gridmsg: gridmsg returned NULL.\n");
	}else{
		buflink_print( b );
		printf("\n");
	}

	buflink_cleanup( b );
	grid_close( g );

	return 0;
}
