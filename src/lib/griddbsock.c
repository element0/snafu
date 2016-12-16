#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <pwd.h>
//#include <unistd.h>

#include "griddbsock.h"

struct sockaddr_un *griddb_generate_sockaddr ( void ) {

	struct sockaddr_un *sa = malloc(sizeof(struct sockaddr_un));
	memset(sa, 0, sizeof(*sa));

	char *s1 = "/var/local";
	char *s3 = "/gridhost/";
	char *s2 = GRIDDBSOCKET_USER;
	char *s4 = GRIDDBSOCKET_BASE;
/*
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

	free(s2);
*/

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

	free(ghostsock);

	return sa;
}
