#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>		/* LOGIN_NAME_MAX */
#include <unistd.h>
#include <pwd.h>

/**
 * TESTED: MONDAY SEP 19, 2016 - Raygan
 * WORKS: Valgrind reports no leaks. (as long as caller frees 'buf').
 * TODO: test buffer overflows
 * This first implementation just wraps the system calls to get effective uid username.
 */
int gridclient_get_username ( void **buf, size_t *len ) {
	struct passwd pwd;
	struct passwd *pwdp;
	size_t pwdrecsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	char *buf2;
	if( pwdrecsize == -1 )
		pwdrecsize = 1024; /*define our own max pwd recsize */
	buf2 = malloc(pwdrecsize);
	if( getpwuid_r( geteuid(), &pwd, buf2, pwdrecsize, &pwdp ) == -1 ) {
		perror("gridclient_get_username: ");
		return -1;
	}
	*len = strnlen(pwd.pw_name, LOGIN_NAME_MAX);
	*buf = strndup(pwd.pw_name, *len);

	free(buf2);
	return 0;
}

int main(int argc, char *argv[] ) {
	char *buf;
	size_t len;
	if( gridclient_get_username( (void *)&buf, &len ) == -1 ) {
		fprintf(stderr, "gridclient_get_username: returned error.\n");
	}else{
		printf("username: %s\n", buf);
	}
	free(buf);

	return 0;
}
