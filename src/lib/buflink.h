#ifndef _SNAFU_PRIM_BUFLINK_H_
#define _SNAFU_PRIM_BUFLINK_H_

#include <stdint.h>
#include <sys/uio.h>

#ifndef GRIDMSGBUFSIZE
#define GRIDMSGBUFSIZE 1024
#endif

/**
 * An iovec array handle.
 */
struct iovarray {
	struct iovec *iov;
	uint64_t *cleanfn;
	int iovcnt;
};
struct iovarray *iovarray_new( int iovcnt );
void iovarray_cleanup( struct iovarray *a );

/**
 * A versatile, linked list buffer for message and reply.
 */
struct buflink {
	char c[GRIDMSGBUFSIZE];
	size_t len;
	struct buflink *next;
};
void buflink_cleanup( struct buflink *buf );
int buflink_print( struct buflink *buf );


#endif /*!_SNAFU_PRIM_BUFLINK_H_*/
