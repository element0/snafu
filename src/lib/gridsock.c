/** @file
 */

#include <sys/types.h>	/* socket may require on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>	/* struct sockaddr_un */

#include <pwd.h>	/* getpwent() */

#include <sys/stat.h>	/* stat(), mkdir() */
#include <unistd.h>	/* stat support, getuid() */	
#include <errno.h>

#include <stdio.h>	/* perror() */
#include <stdlib.h>	/* malloc() */
#include <string.h>	/* memset() */

#include "gridsock.h"

struct sockaddr_un *grid_generate_sockaddr ( void ) {

	struct sockaddr_un *sa = malloc(sizeof(struct sockaddr_un));
	memset(sa, 0, sizeof(*sa));

	char *s1 = "/var/local";
	char *s3 = "/gridhost/";
	char *s2;
	char *s4 = "/local";

	uid_t uid = getuid();
	struct passwd *pwd;

	
	setpwent();
	while((pwd = getpwent()) != NULL)
	{
		if (pwd->pw_uid == uid)
		{
			s2 = malloc(sizeof(char)*strlen(pwd->pw_name)+1);
			strncpy(s2, pwd->pw_name, strlen(pwd->pw_name)+1);
			break;
		}
	}
	endpwent();

	size_t slen = strlen( s1 );
	slen += strlen( s3 );
	slen += strlen( s2 );
	slen += strlen( s4 );
	slen += 1;

	char *ghostsock = malloc(sizeof(char)*slen);
	memset(ghostsock, '\0', sizeof(char)*slen);

	strncat( ghostsock, s1, strlen(s1) );
	strncat( ghostsock, s3, strlen(s3) );
	
	struct stat sb;
	if( stat( ghostsock, &sb ) == -1 )
		if( errno == ENOENT )
			if( mkdir( ghostsock, 0750 ) == -1 )
				perror("mdkir");

	strncat( ghostsock, s2, strlen(s2) );
	if( stat( ghostsock, &sb ) == -1 )
		if( errno == ENOENT )
			if( mkdir( ghostsock, 0750 ) == -1 )
				perror("mdkir");

	strncat( ghostsock, s4, strlen(s4) );

	sa->sun_family = AF_UNIX;
	strncpy(sa->sun_path, ghostsock, sizeof(sa->sun_path) - 1 );

	free(s2);
	free(ghostsock);

	return sa;
}
