#ifndef _SNAFU_GRIDCLIENT_FILEHANDLE_H_
#define _SNAFU_GRIDCLIENT_FILEHANDLE_H_

#include "snafu_ino.h"

/**
 * used to track open files and directories,
 * this structure is pointed to by fuse_file_info member 'fh'.
 * fd[0] write
 * fd[1] read
 */
struct snafu_fh {
	int fd[2];
	size_t size;
	int errornum;
	/**
	 * whether file is empty
	 */
	int empty;
	/**
	 * id to track open and close on serverside of the socket
	 */
	snafu_ino_t id;
};

#endif /*!_SNAFU_GRIDCLIENT_FILEHANDLE_H_*/
