
/*
 * boneyard for grlidclient functions
 */

int grid_stat_axno ( struct cosmos_grid *g, fuse_ino_t ino, struct stat *sb ) {

	struct gridmsg *msg = gridmsg_new( g );

	if( msg == NULL )
		goto abort_before_msg;

	if( gridmsg_set_aframestr( msg, "griddb" ) == -1 ) goto abort_msg;
	if( gridmsg_set_command( msg, "get" ) == -1 ) goto abort_msg;
	if( gridmsg_push_arg( msg, "aframeno=" ) == -1 ) goto abort_msg;

	char itoa[25];	/* 25 places if you want to include commas */
	snprintf( itoa, 25, "%lu", (unsigned long)ino );
	if( gridmsg_append_str( msg, itoa  ) == -1 )
		goto abort_msg;

	gridmsg_print( msg );
	printf("\n");
	void *buf;
	size_t len;
	if( gridclient_send_msg( msg, &buf, &len ) == -1 )
		goto abort_msg;

	struct cosmos_aframe *a;
	if( aframe_at_buffer( &a, buf, len ) == -1 )
		goto abort_msg;

	/* check reported errors */
	switch( aframe_get_error( a ) ) {
		case 0:
			break; /* no error, proceed */
		case ENOENT:
			goto abort_aframe;
		default:
			break;
	}
	if( aframe_stat( a, sb ) == -1 )
		goto abort_aframe;

	return 0;

abort_aframe:
	aframe_cleanup( a );
abort_msg:
	gridmsg_cleanup( msg );
abort_before_msg:

	return -1;
}

int grid_add_url ( struct cosmos_grid *grid, const char *url, const char *url2 ) {
	return 0;
}



struct cosmos_aframe *grid_lookup ( struct cosmos_grid *grid, fuse_ino_t par,
				   const char *name )
{
	fprintf(stderr, "lib/grid_lookup: { grid: %lu, par: %lu, name: %s }\n", (unsigned long)grid, (unsigned long)par, name); 

	return NULL;
}

struct cosmos_aframe *grid_get_aframe_by_ino ( struct cosmos_grid *grid, fuse_ino_t ino ) {
	return NULL;
}





int gridclient_send_msg ( struct gridmsg *msg, void **buf, size_t *len ) {

	if (msg == NULL) return -1;
	
	snafu_ino_t ino = 0;

	char *arg = msg->iov[8].iov_base;
	if (arg[0] == '1' ) { /* root ino */

		arg = msg->iov[10].iov_base;
		if(strncmp(arg, SNAFU_FILENAME, msg->iov[10].iov_len) == 0) {
			ino = 2;
		}
	}

	void *inobuf = malloc(sizeof(snafu_ino_t));
	memcpy(inobuf,&ino,sizeof(snafu_ino_t));

	*len = sizeof(snafu_ino_t);
	*buf = inobuf;

	return 0;
}


