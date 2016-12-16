/** @file
 * 
 * The `griddb` daemon serves primatives to Griduser Daemons.
 * Should run as a trusted user or in a container.
 */

#define _GNU_SOURCE		/* required by 'struct cred' <sys/socket.h> */

#include <sys/types.h>		/* socket requires on some systems, mkdir() */
#include <sys/socket.h>
#include <sys/un.h>		/* struct sockaddr_un */

#include <pwd.h>		/* getpwent() */

#include <sys/stat.h>		/* stat(), mkdir() */
#include <unistd.h>		/* stat support, getuid() */	
#include <errno.h>

#include <stdio.h>		/* perror() */
#include <stdlib.h>		/* malloc() */
#include <string.h>		/* memset() */

#include <regex.h>


#include "griddb.h"
#include "griddbsock.h"
#include "buflink.h"


struct griddb * griddb_init_databases( void ) {

	struct griddb *g = malloc(sizeof(struct griddb));
	if( g == NULL ) {
		fprintf(stderr, "griddb_init_databases: can't malloc griddb.\n");
		return NULL;
	}
	memset(g, 0, sizeof(struct griddb));

	g->vers = 1;

	return g;
}

void griddb_cleanup( struct griddb *g ) {
	if( g == NULL ) return;

	int i;
	for(i=0; i<GRIDDB_NUMDB; i++) {
		if(g->vers == 1) {	
			if(g->db[i] != NULL) {
				kcdbclose(g->db[i]);
				kcdbdel(g->db[i]);
			}
		}
	}

	free( g );
}

/**
 * from bind(2) example
 */
#define handle_error(msg) \
	do{ perror(msg); exit(EXIT_FAILURE); } while(0);

/**
 * Open and Bind the hostwide griddb unix domain socket.
 *
 * unlinks old socket file if it's still around.
 * this should be safe to do, as there should only be one
 * instance of griddb running at a time.
 *
 * TODO: add a check to make sure no other instances are truly running.
 */
static int griddb_open_socket ( struct sockaddr_un *sa ) {

	int sfd;
	sfd = socket( AF_UNIX, SOCK_STREAM, 0 );
	if (sfd == -1)
		handle_error("socket");

	/* remove old socket if it's still around */
	unlink( sa->sun_path );

	/* bind */
	if (bind(sfd, (struct sockaddr *) sa,
				sizeof(struct sockaddr_un)) == -1 )
		handle_error("bind");
	

	return sfd;
}

static void griddb_close_socket ( int sfd, struct sockaddr_un *sa ) {
	if( shutdown( sfd, SHUT_RDWR ) == -1 ) {
		switch errno {
			case EBADF:
			case EINVAL:
			case ENOTCONN:
			case ENOTSOCK:
				perror("griddb_close_socket:shutdown()\n");
		}
	}
	if( unlink( sa->sun_path ) == -1 ) {
		switch errno {
			case EACCES:
			case EBUSY:
			case EFAULT:
			case EIO:
			case EISDIR:
			case ELOOP:
			case ENAMETOOLONG:
			case ENOENT:
			case ENOMEM:
			case ENOTDIR:
			case EPERM:
			case EROFS:
			case EBADF:
			case EINVAL:
				perror("griddb_close_socket:unlink()\n");
		}
	}
}

/**
 * TODO: This needs to produce a user credential object,
 * not just "check".
 */
static void * griddb_get_cred( int sfd ) {
	struct ucred *cred = malloc(sizeof(struct ucred));
	if( cred == NULL ) {
		fprintf(stderr, "griddb_get_cred() can't malloc buffer.\n");
		return NULL;
	}
	socklen_t ucred_len = sizeof(struct ucred);

	if(getsockopt( sfd, SOL_SOCKET, SO_PASSCRED, cred,
				&ucred_len ) == -1)
	{
		perror("getsockopt failed\n");
		return NULL;
	}

	/* MATCH USER ID */
	/*
	uid_t uid = getuid();
	if( uid != cred.uid ) {
		fprintf(stderr, "uid: %d\n", uid);
		perror("yo' cred no good.");
		return NULL;
	}
	*/

	return cred;
}

#define GRIDDB_MSG_MINSIZE 3

/**
 * Interpret message and execute database instruction.
 *
 * Protocol Version 0:
 *	message := version databaseid cmd
 *			keylen delim vallen delim key ':' value ;
 *
 *	version :=  '0';
 *
 *	databaseid := %( [a-z] ) ;
 *
 *	cmd := 'g' | 'p' | 'r' ;
 *
 *	keylen := %( [0-9]+ );
 *
 *	delim  := %( [^0-9] ) ;	// encodes info about key or value
 *
 *	vallen := %( [0-9]+ );
 *	
 *	key    := %<sizeof(keylen)> ;
 *
 *	value  := %<sizeof(vallen)> ;
 *
 * @param grdb grid database collection object
 * @param uc user credentials
 * @param mbuf message buffer
 * @return returns a `struct buflink` linked buffer list of data
 */
static void * griddb_msg_exec ( struct griddb *g, void *uc, struct buflink *mbuf ) {

	if( g == NULL ) {
		fprintf(stderr, "griddb: griddb is NULL.\n");
		return NULL;
	}
	if( mbuf == NULL ) {
		fprintf(stderr, "griddb: message is NULL.\n");
		return NULL;
	}
	

	// does message have adequate length? what about girth?

	if( (mbuf->len < GRIDDB_MSG_MINSIZE) )  {
		fprintf(stderr, "griddb: message less than min size.\n");
		return NULL;
	}
	
	// identify protocol make sure its bone 0

	char *mp = (char *)mbuf;

	size_t pos = 0;

	int protno = mp[pos++] - 48;

	if( protno != 0 ) {
		fprintf(stderr, "griddb: only supports version 0 messages.\n");
		return NULL;
	}
	


	// locate or create the database
	// must be a-z

	int dbid = mp[pos++];

	if( dbid < 97 || dbid > 122 ) {
		fprintf(stderr, "griddb: dbid must be [a-z].\n");
		return NULL;
	}

	int dbno = dbid - 97;

	// create and open if needed, otherwise just get db handle

	KCDB *db;
	if( g->db[dbno] == NULL ) {
		db = g->db[dbno] = kcdbnew();
		char dbfilename[] = "griddb-.kch";
		dbfilename[6] = dbid;

		int dbfilepathlen = strlen(GRIDDBSOCKET_DIR) +
			+ 1 + strlen(GRIDDBSOCKET_USER) + 1 +
			strlen(dbfilename) + 1;

		char *dbfilepath = malloc(sizeof(char)*dbfilepathlen);
		if( dbfilepath == NULL ) {
			fprintf(stderr, "can't allocate dbfilepath.\n");
			return NULL;
		}

		snprintf(dbfilepath, dbfilepathlen,
				GRIDDBSOCKET_DIR "/" GRIDDBSOCKET_USER "/%s",
				dbfilename);

		if( !kcdbopen(db, dbfilepath, KCOWRITER | KCOCREATE ))
		{
			fprintf(stderr, "can't create database: ");
			fprintf(stderr, "%s\n",kcecodename(kcdbecode(db)));
			free(dbfilepath);
			kcdbdel(db);
			g->db[dbno] = NULL;
			return NULL;
		}

		free(dbfilepath);

	}else{
		db = g->db[dbno];
	}


	// parse command, key and value

	char cmd = mp[pos++];
	char *curpos = &mp[pos];

	char *nexpos;
	size_t klen = strtol(curpos, &nexpos, 10);

	curpos = nexpos+1;
	size_t vlen = strtol(curpos, &nexpos, 10);

	curpos = nexpos+1;
	char *key = curpos;
	char *sep = curpos+klen;
	char *val = sep+1;
	char *end = val+vlen;

	sep[0] = '\0';
	end[0] = '\0';

	// prep return buffer
	struct griddb_rmsg *rmsg = malloc(sizeof(*rmsg));
	memset(rmsg,0,sizeof(*rmsg));

	// execute cmd
	switch( cmd ) {
		// set
		case 's':
			if( !kcdbset(db, key, klen, val, vlen) ) {
				fprintf(stderr, "can't set record: %s\n",
						kcecodename(kcdbecode(db)));
			}
			rmsg->code[0] = 'O';
			rmsg->code[1] = 'K';
			break;
		// get
		case 'g':
			val = NULL;
			val = kcdbget(db, key, klen, &vlen);
			if (val == NULL) {
				fprintf(stderr, "can't get record: %s\n",
						kcecodename(kcdbecode(db)));
			}
			rmsg->val     = val;
			rmsg->vlen    = vlen;
			rmsg->cleanfn = kcfree;
			break;
		// remove
		case 'r':
			if( !kcdbremove(db, key, klen) ) {
				fprintf(stderr, "can't remove record: %s\n",
						kcecodename(kcdbecode(db)));
			}
			break;
	}
	return rmsg;
}

void griddb_main_loop ( struct griddb *g ) {

	/* socket address */
	struct sockaddr_un *sa = griddb_generate_sockaddr();

	/* open socket */
	int sfd = griddb_open_socket ( sa );
	
	/* listen */
	if (listen(sfd, LISTEN_BACKLOG) == -1)
		handle_error("listen");

	/* accept loop */

	int cfd;
	struct sockaddr_un peer_addr;
	memset(&peer_addr, 0, sizeof(struct sockaddr_un));
	socklen_t peer_addr_size = 0;

	void *uc;

	while ( (cfd = accept( sfd, (struct sockaddr *) &peer_addr,
					&peer_addr_size)) != -1 )
	{

		/* credentials */

		if( (uc = griddb_get_cred( cfd )) == NULL )
			handle_error("credentials");

		/* read msg */

		struct buflink *buf = malloc(sizeof(struct buflink));
		if( buf == NULL )
			handle_error("can't malloc input buffer.\n");
		memset(buf, 0, sizeof(struct buflink));

		struct buflink *cur = buf;
		while( cur != NULL ) {
			cur->len = read(cfd, (void *)cur->c, GRIDMSGBUFSIZE-1);
			if( cur->len == GRIDMSGBUFSIZE-1 ) {
				cur->next = malloc(sizeof(struct buflink));
			}
			cur = cur->next;
		}


		/* execute command */
		struct griddb_rmsg *rmsg = griddb_msg_exec( g, uc, buf );


		/* return result */	
		if( rmsg != NULL ) {
			if(rmsg->code[0] != '\0') {
				write( cfd, rmsg, 2 );
			}
			write( cfd, rmsg->val, rmsg->vlen+1 );
			fsync(cfd);

			if(rmsg->cleanfn != NULL )
				rmsg->cleanfn( rmsg->val );
		}else{
			printf("return NULL.\n");
		}
		
		/* cleanup */
		buflink_cleanup( buf );

		close(cfd);
	}
	
	/* cleanup the socket pathname using unlink(2) or remove(3) */
	shutdown( cfd, SHUT_RDWR );
	griddb_close_socket( sfd, sa );
	free(sa);
}

