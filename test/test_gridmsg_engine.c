#include <stdio.h>

#include "../src/lib/gridmsg_engine.h"

int main(int argc, char *argv[]) {
	if( argc != 2) {
		printf("usage: test_gridmsg_engine <MSGSTRING>\n");
		return -1;
	}

	gridmsg_exec( NULL, (void *)argv[1] );

	return 0;
}
