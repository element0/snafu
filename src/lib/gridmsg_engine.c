/*
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 */

/** @file
 * Implements Gridmsg Parser and Gridmsg Router
 *
 * Todo Rename "gridengine.c"
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include "cosmos_grid.h"	
#include "buflink.h"
#include "griddb_client.h"


/**
 * Grid Message Protocol (Griduser Socket)
 * Draft 1
 */
#define MSG_SCHEME "[a-z]+"
#define MSG_USER   "[[:alnum:]_.]+"
#define MSG_HOST   "[[:alnum:]_.]+"
#define MSG_PATH   "[^ ]+"
#define MSG_CMD    "[[:alnum:]/._-]+"
#define MSG_ARGS   ".+"

#define MSG_REGMATCH 25


/**
 * Parse and Route
 *
 * ROUTE: to Griddb
 * - translates the message into a griddb message
 * - sends message to griddb
 * - returns the result
 *
 * ROUTE: to local Gridhost (self)
 *
 * ROUTE: to peer Gridguest over ssh
 */
void *gridmsg_exec( struct cosmos_grid *g, struct buflink *buf ) {

	struct buflink *rbuf = NULL;

	/* PARSER */

	/* Parse Incoming Message */
	char *repat = "((" MSG_SCHEME "):((//)((("MSG_USER")@)?("MSG_HOST"))?)?(/)?)(("MSG_PATH")?([ ]+("MSG_CMD")([ ]+("MSG_ARGS"))?)?)?$";


	enum { MSGIND_ALL, MSGIND_HEAD_REQ, MSGIND_SCHEME,
	       MSGIND_USERHOST_AREA_OPT,
	       MSGIND_DBLSLASH_OPT,
	       MSGIND_USERHOST_OPT,
	       MSGIND_USER_OPT,
	       MSGIND_USER, MSGIND_HOST,
	       MSGIND_SLASH_OPT,
 	       MSGIND_BODY_OPT, 
	       MSGIND_PATH,
	       MSGIND_CMDLINE_OPT,
	       MSGIND_CMD, MSGIND_ARGS_OPT, MSGIND_ARGS };

	regex_t msgreg;
	memset(&msgreg, 0, sizeof(regmatch_t));
	regcomp(&msgreg, repat, REG_EXTENDED);


	regmatch_t match[MSG_REGMATCH];
	memset(match, 0, sizeof(regmatch_t)*MSG_REGMATCH);

	regexec(&msgreg, (char *)buf, MSG_REGMATCH, match, 0);
	regfree(&msgreg);


	if( match[MSGIND_HEAD_REQ].rm_eo == 0 ) {
		fprintf(stderr, "gridmsg_exec: doesn't meet minimum \"<scheme>:\"\n");
		return NULL;
	}

	char *schemeptr = "";
	int  schemelen  = 0;
	if( match[MSGIND_SCHEME].rm_so != -1 ) {
		schemeptr = (char *)buf + match[MSGIND_SCHEME].rm_so;
		schemelen = match[MSGIND_SCHEME].rm_eo - match[MSGIND_SCHEME].rm_so;
		printf("scheme.rm_so: %d\n", match[MSGIND_SCHEME].rm_so);
		printf("schemelen: %d\n", schemelen);
	}

	char *pathptr = "";
	int  pathlen  = 0;
	if( match[MSGIND_PATH].rm_so != -1 ) {
	    pathptr = (char *)buf + match[MSGIND_PATH].rm_so;
	    pathlen = match[MSGIND_PATH].rm_eo - match[MSGIND_PATH].rm_so;
	    printf("pathlen: %d\n", pathlen);
	}

	char *cmdptr = "";
	int  cmdlen  = 0;
	if( match[MSGIND_CMD].rm_so != -1 ) {
		cmdptr    = (char *)buf + match[MSGIND_CMD].rm_so;
		cmdlen    = match[MSGIND_CMD].rm_eo - match[MSGIND_CMD].rm_so;
		printf("cmdlen: %d\n", cmdlen);
	}

	/* two arguments */
	memset(&msgreg, 0, sizeof(regmatch_t));
	regcomp(&msgreg, "([^ ]+)([ ]+(.+))?", REG_EXTENDED);
	regmatch_t argmatch[4];
	memset(argmatch, 0, sizeof(regmatch_t)*4);

	char *args = (char *)buf + match[MSGIND_ARGS].rm_so;
	printf("args: %s\n", args);

	regexec(&msgreg, args, 4, argmatch, 0);
	regfree(&msgreg);


	char *argptr1 = "";
	int  arglen1 = 0;
	if( argmatch[1].rm_so != -1 ) {
		argptr1   = args + argmatch[1].rm_so;
		arglen1   = argmatch[1].rm_eo - argmatch[1].rm_so;
		argptr1[arglen1] = '\0';
		printf("arglen1: %d\n", arglen1);
		printf("argptr1: %s\n", argptr1);
	}

	char *argptr2 = "";
	int  arglen2 = 0;
	if( argmatch[3].rm_so != -1 ) {
		argptr2   = args + argmatch[3].rm_so;
		arglen2   = argmatch[3].rm_eo - argmatch[3].rm_so;
		printf("arglen2: %d\n", arglen2);
		printf("argptr2: %s\n", argptr2);
	}

	/* ROUTER */

	/* Select Context of Message */

	/* griddb:[a-z] */
	int  protocol_num = 0;
	char db = pathptr[0];
	char cmd = cmdptr[0];
	char ksiz[50];
	snprintf(ksiz, 49, "%d", arglen1);

	char vsiz[50];
	snprintf(vsiz, 49, "%d", arglen2);

	char sepcode1 = 'k';
	char sepcode2 = 'v';
	char *key = argptr1;
	char *val = argptr2;


	/* ROUTE: to Griddb */
	/* griddb: */

	if( strncmp( schemeptr, "griddb", schemelen) == 0 ) {

		rbuf = griddb_msg( g,
				protocol_num,
				db,
				cmd,
				ksiz,
				sepcode1,
				vsiz,
				sepcode2,
				key,
				val );
	}

	/* ROUTE: to Local Gridhost */
	/* file://localhost/ servermod */
	/* file://localhost/ lookupmod */

	char *errstr;
	char *inoid;
	if( strncmp( schemeptr, "griduser", schemelen) == 0 ) {
		pathptr[pathlen] = '\0';
		printf("query griduser database: %s\n", pathptr);

		// griduser stat
		if(( cmdlen > 0 )&&( strncmp( cmdptr, "stat", cmdlen ) == 0 ))
		{
			if( arglen1 > 0 ) {
				inoid = argptr1;
				griddb();
			}else{
				errstr = "ER: stat needs an ino.";
				rbuf = malloc(sizeof(*rbuf));
				memset(rbuf, 0, sizeof(*rbuf));
				rbuf->base = strndup(errstr, strlen(errstr));
			}
			
		}
	}
	/* ROUTE: to Peer */



	return rbuf;
}
