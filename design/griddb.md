Griddb {#griddb}
======

[TOC]

*UPDATE:* the Griddb is an abstract database shared between Protected User domain and Shared Multiuser domain.  Each storage element is routed based on ACL.

The Griduser Daemon and a host-wide Gridshare Daemon have identical database capabilities.

The Gridshare Daemon doesn't have the other functions of Griduser Daemon, but it has a strict ACL guard and User Authenticator.  It doesn't connect to the network but uses a Unix Domain socket to provide service to Griduser Daemon clients on the local machine.

Gridshare runs as a trusted user or as root inside of a container.

*NOTE:* an immediate benifit of hiding the separation of Single and Multiuser space behind the Griddb API is the first draft implementation need only implement the Single User side.



\section griddb_overview	Griddb Overview

The Griddb keeps databases of Cosmos file system primatives.

- aframes
- dcels
- fcels	(fragments)
- mcels (map cells)
- production instructions (as gridmsg's)
- addresses
- bundles
- hiena grammar machines
- string id's
- rootstr
- btree nodes
- ACL's

An example of why these databases need to be kept in multi-user space:  Take for instance the root of the local filepath:

	snafu://localhost/

This local aframe is global to all users of the localhost.



\section  db_arch	Griddb Architecture

The Griddb runs one-per-host.  One Griddb serves multiple local users.  It does not serve network users except those who have logged in through ssh.

	griddb server

The Griddb is available over a local socket.  As such, Griddb Clients must connect by using messages.

	griddb client library
	
	griddb socket

The Griddb process authenticates each connection using local authentication, such as credentials passing over the socket.

	user credentials authentication

The Griddb interprets the message on the databse subject to ACL's and returns appropriate data.

	ACL's

The Griddb manages a database backend and stores data in a protected area of the local server.

	backend database library



\section  db_implem	Database Implementation

First Draft implementation:

Kyoto Cabinet Library 
Kyoto Cabinet databases

Griddb Daemon accepts connections on Griddb Socket.

Authentication:		SKIP IMPLEMENTATION

ACL's:			SKIP IMPLEMENTATION
{Database ID, Element Key} pairs are used to identify ACL's.

Griddb interprets Griddb Message to Kyoto Cabinet function call.

Returns result.



\section  aframedb_setup   Setting Up the Aframes-based Griddb

The databases are organized in [aframes](@ref accessframe).

The Griddb database aframes can be set up programmatically.  We can do this through a single comprehensive function:

	griddb_init_databases();

We have a different source file for each database:

	src/lib/griddb_stringsdb.[hc]

Each database has an init function which copies in its own functions to the `aframe/cosm/bin` locations.  And which initializes the `aframe/fs` object as a database or a dcel directory structure.

	%{dbname}_init( struct aframe *a );

The griddb aframes are linked into a directory structure from a container aframe.  This directory structure uses the lookup_cache inside the container to identify each child database.

	struct aframe *dbroot = aframe_new();

	aframe_add_branch_cache( dbroot, dbnumber, dbaframe );

The container is stored in memory as `dbroot`.



\section  griddb_msg	Griddb Messages

Griddb messages begin with a protocol number.

	griddb_msg := protocol_num message_stream

Thereafter, message_stream is defined by the grammar of the protocol version.



\subsection griddb_msg_draft1	First Draft Protocol

The first draft protocol tries to match `kclangc.h` API definitions of `set` and `get` as closely as possible, but arranged for serial parsing -- ie, the size arguments come before the values.

The first draft protocol is as follows:

	message_stream := dbid "set" ksiz key vsiz value 
			| dbid "get" ksiz key vsiz
			| dbid "rm"  ksiz key

Plain text examples:

	string set 57 "hello there my name is hotdogboy."

(psuedo-)Encoded examples: (mixing hex and ascii)

	Ss0x040x390x21hello there my name is hotdogboy.

Where each database gets a single capital letter to represent it.

	S = stringdb
	A = aframes
	D = dcels
	...

And where each command gets a single lowercase letter.

	s = set
	g = get
	r = rm



PREVIOUS WORK... (imported from "gridclient.md")

\section grid_databases		Grid Databases


A database is created implicitly by any of the access methods.

Array methods identify records by database name and index number.

	grid_get( gh, dbname, indexno );

Key-value methods identify records by database name and key string.

	grid_kvget( gh, dbname, key_str );

Query methods identify records by database name and query string.

	grid_query( gh, dbname, query_str );

Pathname methods identify records by database name and lookup string.

	grid_lookup( gh, dbname, lookup_str );

List methods step from record to record.

	grid_step( gh, gridrecord );




\section database_modes 	Database modes


Dictionary: every element is uniq.

Arrays: every element can be the same.

Filepath: elements are stored in a directory structure.  Each dirent must be uniq.




