#ifndef _SNF_SUITE_PATH_H_
#define _SNF_SUITE_PATH_H_

#include <sys/stat.h>
#include "snf_suite_ino.h"


int snf_stat_path( struct cosmos_grid *g, snafu_ino_t ino, struct stat *sb );


#endif /*!_SNF_SUITE_PATH_H_*/
