#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buflink.h"

struct iovarray *iovarray_new ( int iovcnt ) {
	struct iovarray *a = malloc(sizeof(struct iovarray));
	if( a == NULL ) return NULL;
	memset(a,0,sizeof(struct iovarray));

	a->iov  = malloc(sizeof(struct iovec)*iovcnt);
	memset(a->iov,0,sizeof(struct iovec)*iovcnt);

	a->cleanfn = malloc(sizeof(uint64_t)*iovcnt);
	memset(a->cleanfn,0,sizeof(uint64_t)*iovcnt);
	return a;
}
/**
 * the case may be that a->iov[n] points to an offset inside of a malloc'd
 * region. if a->hint[n] = 'f' it needs to be free'd. 
 */
void iovarray_cleanup ( struct iovarray *a ) {
	if(a == NULL) return;
	if(a->iov != NULL) {
		if(a->cleanfn != NULL) {
			int n;
			for(n=0; n < a->iovcnt; n++) {
				if((void *)a->cleanfn[n] != NULL) {
					void (*cleanfn)( void * )
						= (void *)a->cleanfn[n];
				
					cleanfn(a->iov[n].iov_base);
				}
			}
			free(a->cleanfn);
		}
		free(a->iov);
	}
	free(a);
}

void buflink_cleanup( struct buflink *buf ) {
	struct buflink *cur = buf;
	struct buflink *next;

	while( cur != NULL ) {
		next = cur->next;
		free( cur );
		cur = next;
	}
}
int buflink_print( struct buflink *buf ) {
	struct buflink *cur = buf;
	while( cur != NULL ) {
		printf(cur->c);
		cur = cur->next;
	}
	return 0;
}

