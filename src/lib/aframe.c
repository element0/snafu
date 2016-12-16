/* 
 * Cosmos::Aframe Implementation
 */
#include <stdlib.h>
#include <string.h>
#include "aframe.h"


struct cosmos_aframe *aframe_new() {
	struct cosmos_aframe *a = malloc(sizeof(struct cosmos_aframe));
	if( a != NULL)
		memset(a, 0, sizeof(struct cosmos_aframe));
	return a;
}

void aframe_cleanup( struct cosmos_aframe *a ) {
	free(a);
}


/**
 * Typecase an aframe from a buffer pointer.
 * supposedly this buffer contains an aframe pattern
 * test the buffer for aframe.
 * use the buffer address don't duplicate the buffer.
 * 
 * Aframe will be required to dispose of buffer.
 */
int aframe_at_buffer( struct cosmos_aframe **a, void *buf, size_t len ) {
	if( buf == NULL) return -1;

	/* test for aframe */
	/* TODO: improve this */
	if( len != sizeof(struct cosmos_aframe) ) {
		return -1;
	}

	*a = buf;

	return 0;
}


int aframe_stat ( struct cosmos_aframe *aframe, struct stat *buf )
{
	/* TEMP FILL STAT BUFFER WITH DIRECTORY INFO */
	if (buf == NULL) return -1;
	
	memcpy(buf, &aframe->sbuf, sizeof(struct stat));
	
	return 0;
}

int aframe_get_error( struct cosmos_aframe *aframe ) {
	if( aframe == NULL ) return 0;
	return aframe->error;
}
int aframe_readdir_r( struct cosmos_aframe *aframe, struct dirent *entry, struct dirent **result ) {
	return 0;
}
uint64_t aframe_fopen( struct cosmos_aframe *aframe, int mode ) {
	return 0;
}
size_t aframe_fread( char *buf, size_t size, uint64_t fh ) {
	return 0;
}
int aframe_fclose( uint64_t fh ) {
	return 0;
}
