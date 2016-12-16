Griduser Daemon  {#griduserd}
===============

Griduser Daemon runs one per user, per host.  It's job is as follows:

- read local connections
- maintain Grid Topology
- maintain "guests" on Grid Topology hosts
- dispatch messages to guests
- execute local destination messages
- maintain Source Server modules
- maintain Lookup module


Keeps these databases:

- lookup modules
- server modules
- grid topology
- gridmsg delegation frames
- lookup strings
- non-shared user data


Uses the Griddb:

- Grid Topology store (maximum is per directory in filetree)
- File System Primatives



\section  griduser_proof	Proof of Concept Drafts

Priorities for Proof of Concept

First Draft:

- read local connections
- execute local destination messages
- maintain Source Server modules
- maintain Lookup module

Second Draft:

- maintain Grid Topology
- maintain "guests" on Grid Topology hosts
- dispatch messages to guests



\section  griduser_walkthrough		Walkthrough


*STARTUP*

The griduser daemon is started by the first gridclient to try to connect to the griduser socket.


*FUSE SESSION*

A FUSE based volume uses the Gridclient Library to setup its mountpoint.

There is a sequence of necessary events to prepare an `access frame` which will be linked to the mountpoint.  (Every `ino` from the FUSE volume will reference an `access frame`.)

-# Build local `Access Path` to Mountpoint
-# Preserve Underlying Mountpoint


### Build Local `Access Path` to Mountpoint ###

Cosmos provides a simple interface to acquire an `access frame` to any URI formed as a `cosmos` (formerly `snafu`) scheme URI.

	cosmos:///home/redboy/project

Any such URI will invoke a pathwalk in which each segment will generate an `access frame` with a `cosm`.  (The `cosm` will be interpreted for `types` and `flows` among otherthings.  The `flows` defines the Grid Topology of each `access frame`.)

As each `access frame` has a `dcel`, and each `dcel` has a set of `fragments` which describe the sources of the domain.  A fragment has a `production instruction` (`generative grammar`) which leads directly or indirectly to a `source production instruction` with the local filesystem component.

	file:///home/redboy/project

This source should be preserved before the FUSE volume binds to the mountpoint through the host VFS.


### Preserve Underlying Mountpoint ###

Griduser Daemon calls "preserver" server with `file` scheme URI.

	> ./serverlib/preserver --preserve "file:///home/redboy/project"

Then the Griduser Daemon swaps the "file" module with the "preserver" module 

(Later when the Volume unmounts... )

	> ./serverlib/preserver --release ${uri}








\section  server_modules_db	Server Modules DB


Griduser Daemon loads server modules as needed.


	server_db
	{
		server[]
	}


	server
	{	
		op[]
	}

	op
	{
		open
		close
		read
		write
		seek
		stat
		rewind
		truncate
		insert
		append
		unlink
	}


