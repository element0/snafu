Architecture  {#architecture}
============

[TOC]

The basic architecture is defined by the following needs.

	- multiple user-processes can use the grid at once
	- each user's grid processes run in userspace
	- connections between hosts are per user in userspace
	- local, multi-user database coordinates shared data
		- but does not connect to the network

A user can have multiple processes running in userspace which connect to the grid using a gridclient library.

	/-------------------------------------------------------\
	|    user proc   |    user proc 2   |    user proc 3    |
	|-------------------------------------------------------|
	|                 gridclient library                    |
	|-------------------------------------------------------|
	|               local socket (per user)			|           
	|-------------------------------------------------------|
	|                 griduser daemon (per user)            |
	| calculates:
	|	Grid Topology
	|	Deligation Frame
	|	Grid Messages
	|
	| modules loaded per user:
	|	Source Server Modules
	|	Lookup Module
	|	Hiena Engine Module
	|	
	|-------------------------------------------------------|
	|  grid network connection  |  grid database connection |
	|-------------------------------------------------------|
	|    remote guest           |       shared griddb       |
	|---------------------------| cached per all:		|
				    |   Grid Topology		|
				    |   Hiena Types		|
				    |   File System Primatives	|
				    \---------------------------/



[Snafu FUSE Volume][sf] uses [Gridclient Library][lg].

Gridclient Library uses [Grid Messages][gm] over socket to Griduser Daemon.

Griduser Daemon:
1) calculates [Grid Topology][gt] dispatches Grid Messages over ssh pipes to remote guest Griduser Daemons.
2) uses [Griddb Library][db]
3) uses [Source Server Modules][srv]
4) uses [Lookup Module][lm]
5) uses [Hiena Engine Library][h] with Hiena Grammar Machine Objects.

[Server modules][srv] access external servers and keep address translation tables.

[Griddb Library][db] uses Griddb Messages over socket to Griddb Primatives Daemon.

Griddb Primatives Daemon stores and retrieves file system primatives.

[sf]:@ref snafuvol
[lg]:@ref gridclient
[gm]:@ref gridmessage
[gu]:@ref griduserd
[gt]:@ref gridtopology
[db]:@ref griddb
[srv]:@ref servermod
[lm]:@ref lookupmod
[h]:@ref hiena


\section source_code_architecture	Source Code Architecture

	snafu FUSE volume
		<fuse_lowlevel.h>
		gridclient
			gridsocket
			gridmsg

	griduser host daemon
		gridengine
			gridmsg_parser
			gridrouter
				griddbclient
					griddbsocket
					griddbmsg
				gridhost
					server modules
					<hiena.h>
					<cosmos.h>
				gridguest_client
					ssh
	gridguest daemon
		gridclient
			gridsocket
			gridmsg
	
	griddb database daemon
		griddbsocket
		griddbmsg_parser.h
		<kcdbc.h>

\section install_layout		Installation File Layout

	
