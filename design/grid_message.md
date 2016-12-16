Grid Message	{#gridmessage}
============

[TOC]

This document explores the `gridmsg` protocol used between `gridclient` programs and `griduser` service.

The `griduser` process runs in userspace.  It listens to a `AF_UNIX` domain socket on the localhost.  It receives messages from other processes on the localhost who have permission to use the socket.

The `gridhostd` allows multi-user support.  Runs either in userspace or privilidged.  It keeps the `griddb` databases.  The databases contain ACL information for each entry.

Both griduser and gridhostd communicate over sockets.  Griduser accepts connections from Gridclient programs.  Gridhostd accepts connections from Griduser programs. 

The `gridmsg` format is used over the `gridclient`->`griduser` socket.


**Grammar**

A `gridmsg` follows this basic form:

	gridmsg := context command

Examples:

	griddb://bangfish.local/axpa get 0012

The above context is `griddb://bangfish.local/axpa` and the command is `get 0012`.

	griddb://bangfish.local/axpa new
	griddb://bangfish.local/rootstr set "snafu://bangfish.local" 12



**Command Examples**

Each context may have its own set of commands.  So technically there is a different dictionary per context.

What I will describe here are the contexts and commands necessary to implement the Snafu FUSE file system backend.

I'll begin with a playround of messages in psuedocode:

A `gridclient` sends messages to `griduserhost`:

Mental break ---

This is what I want to happen.

	mountpoint = "localhost/home/redboy/project"

first we build a filetree of the local `mountpoint`.

	$ cur = root = access_frame.new( "file://localhost/" )
	$ cur.cosm = cosmos.buildcosm( cur )
	$ cur  = lookup( cur, "home" )	      // lookup uses cur.cosm.grid
	$ cur.cosm = cosmos.buildcosm( cur )
	$ cur  = lookup( cur, "redboy" )      // lookup uses cur.cosm.grid
	$ cur.cosm = cosmos.buildcosm( cur )
	$ cur  = lookup( cur, "project" )      // lookup uses cur.cosm.grid
	$ cur.cosm = cosmos.buildcosm( cur )

at this point the contents of `cur` might not equal the contents at `localhost/mountpoint` because the contents have been influenced by `cur.cosm.grid`.

but what we want is a handle on the access frame at `mountpoint` because we want to use that as the content of our snafu volume.  so what we've done is useful.  what we've got right now is:

	$ snafu_volume.root_access_frame = cur

But we want to preserve the underlying contents from being unavailable after we mount over with fuse.  we can get that if we use the underlying operating system to give us a handle on the `mountpoint` and then save that handle.

	$ handle = preservation_server_init( mountpoint );

So now we have a handle, but we have to get it to stick inside the composition of the `root_access_frame`.  The root access frame has a `dcel` which represents the domain of data combined from the grid.

The dcel has a `production instruction` called `bind` which has a number of arguments, each a dcel which represent a component of data to be combined.  One of these component dcel's will represent the `mountpoint`.  It should be swapped out with a dcel which represents the preservation `handle`.

	$ dcel = dcel.produce.source( handle )

--
we have to jump back and see how the lookup function created the access frame in the first place.

	$ aframe = access_frame.new( "file://localhost/" )
	==
	$ aframe = access_frame.proto()
	$ dcel = dcel.produce.source( "file://localhost/" )
	$ aframe.set_dcel( dcel )

	$ cosm = cosmos.buildcosm( aframe )
	$ aframe.set_cosm( cosm )
	$ aframe.cosm.grid.print()
	> localhost
	> remotehost


...
SLEEPY TIME...

DING!

<wake up>

-- -- -- -- BEGIN: 10-14-2016 RH  -- -- -- --

here's the plan:

Gridclient can't be aware of the complexities of the grid.  So the "snafu://" scheme is a no-go for messages from gridclient.  A Gridclient merely interacts with the grid via customary URI standards.  Namely, "file://", "ssh://", "https://", "sftp://", etc.

A snafu FUSE mounter need only look like this:

	snafu file://bangfish.local/home/raygan/nuts <mountpoint>

	-or simply-

	snafu ~raygan/nuts

	-or remotely-

	snafu ssh://wildone.kamenet/home/stargas/nuts <mountpoint>

Snafu's CLI syntax is defined as follows:

	snafu [ SOURCEURI ] MOUNTPOINT

If there is no `SOURCEURI` then the `MOUNTPOINT` doubles as one.

When the `mountpoint` doubles as source, the existing file tree at that point must be preserved (see [Bind Mount](@ref bindmount)).

Internally inside the `snafu.c` code we use the standard URI format to interact with the `grid` API.

	ino = gridmsg(g, "file://bangfish.local/home/raygan/nuts" );

Could get an ino from the grid that can be used to interact with the data at that location.

However, where I'm pushing this, is that the _way_ the URI is _processed_ by the grid, merges each segment's distributed storage into the directory structure as the URI path is walked.  Therefore a `file://` scheme URI submitted to `firefox` and the same one submitted to `gridmsg` might return different content.

*The grid processes the URI to find and merge distributed content with local content.*

Two different uses of the same URI:
- targeting local storage ( typical file scheme expectation )
- targeting local and distributed storage ( snafu grid magic )

(which is why I'm riding the fence on a "snafu://" scheme.  because it implies both local and distributed storage, whereas, "file://" really only implies local.)

The `gridclient` API should clearly enable both.

	ino = grid_expand_uri(g, snafuri);
	ino = grid_normal_uri(g, fileuri);

--

The `gridclient` API protects access to a local URI.

	ino = grid_preserve_local_uri(g, fileuri);
	err = grid_unpreserve_local_ino(g, ino);

--

binds local URI to distributed

	ino = grid_bind_uri(g, fileuri, snafuri);

--

The `gridclient` API then has all those methods for tracking ino's as FUSE requires.

	ino = grid_lookup_ino(g, ino, string);
	sbuf = grid_stat_ino(g, ino);
	fd  = grid_open_ino(g, ino);
	fd  = grid_opendir_ino(g, ino);
	err = grid_releasedir_ino(g, ino);
	err = grid_close_ino(g, ino);

--

The `griduser protocol` must implement the above.

	ino = grid_expand_uri(g, uri);
	==
	gridmsg(g, "%s %s", uri, "snafu::expand");

	ino = grid_normal_uri(g, uri);
	==
	gridmsg(g, "%s %s", uri, "server::access");

	ino = grid_preserve_normal_uri(g, uri);
	==
	gridmsg(g, "%s %s", uri, "server::preserve");

	ino = grid_lookup_ino(g, ino, string);
	==
	gridmsg(g, "%lu %s %s", ino, "ino::lookup", string);

	sbuf = grid_stat_ino(g, ino);
	==
	gridmsg(g, "%lu %s", ino, "ino::stat");

	fd = grid_open_ino(g, ino)
	==
	gridmsg(g, "%lu %s", ino, "ino::open");

	... (and so forth)

--

The `griduserhost daemon` would receive the following input strings.

	file:///home/raygan/nuts snafu::expand
	file:///home/raygan/nuts server::access
	file:///home/raygan/nuts server::preserve
	12 ino::lookup redboy
	13 ino::stat
	13 ino::open

These might as well be formatted like typical unix commands.

	snafu --expand file:///home/raygan/nuts
	server --access file:///home/raygan/nuts
	server --preserve file:///home/raygan/nuts
	ino --lookup 12 redboy
	ino --stat 13
	ino --open 13

So the protocol API might as well look like:

	gridmsg(g, "%s %s", "snafu --expand", uri);
	gridmsg(g, "%s %s", "server --access", uri);
	gridmsg(g, "%s %s", "server --preserve", uri);
	gridmsg(g, "%s %lu %s", "ino --lookup", ino, string);
	gridmsg(g, "%s %lu", "ino --stat", ino);
	gridmsg(g, "%s %lu", "ino --open", ino);

--

And thereabove, the API is simple.  The command protocol is simple.  The complexity is left to the `griduserhost daemon`.



-- -- -- -- END 10-14-2016 RH -- -- -- --
--

\section snafu_scheme	Snafu URI Scheme

Lets say the "redboy" directory contains a ".cosm/flows" configuration:

	bangfish.local == flyboy.local

which means "/home/redboy" should be equal on both hosts.  so the Snafu Suite binds both sources for the dcel at the "redboy" access path.  Thus, the lookup began with a "file://" URI, but during the pathwalk, it branched to "ssh://flyboy.local/home/redboy".

(This could be a case for changing the "scheme" from "file" to "snafu".  I like "file" because the root is clearly local.  However, snafu indicates that entire path need not be local to be valid.
 
As "bangfish.local/" can mean something different for different schemes, the actual root table entry needs to start with the scheme name.  "file://bangfish.local/" is different from "https://bangfish.local/" )

As much as I hate the overpopulation of the ICANN scheme name registry, and I have strong opinions about what a scheme name should be used for and what it should not, here are some reasons why I think a "snafu" scheme name _could_ be appropriate.

I want to use file scheme URI's because they imply host locality.  But even a file scheme URI can include authority and host, though I don't know of any examples of it.  So conceivably, a file URI with a hostname would imply remote access.  Though it would be up to the user to decide how.

A snafu scheme URI could inherit the file schemes expectations about hostname.  But the snafu scheme name would A) alert users to the fact that each path segment might have a different distribution on the grid and B) that each path segment might not really be a file proper, but a calculated sub-area of a file, mapped into the file tree.  These two reasons are why I think using "file" scheme URI's is misleading for being used with the snafu suite.

A file scheme URI would of course overlap, if used through a web browser to locate data on a Snafu volume.  Because at that point, the file URI would simply indicate an area within the host's filespace.  The snafu URI's would already be mapped into the host's filespace and therefor be a valid file.

To summarize:
- snafu scheme URI is for use with the Snafu Suite _before_ being mapped into the host filespace.
- file scheme URI is for use with the host OS _after_ the Snafu file system has been mapped into the host filespace.


OLD WORK IN PROGRESS...

first we preserve access to the underlying file system located at the mountpoint.

	mountpoint = localhost/home/redboy/project
	
	dno  = $(preserve file:///home/redboy/project)

	ino  = $(snafu expand snafu:///home/redboy/project)

	aframe = $(aframe --get ${ino})

	aframe.dcel.bind( dno );

	err = $(aframe --set ${ino} ${aframe})

	ino = $(ino --lookup ${ino} "redzone")

	ino = $(ino --lookup ${ino} "subzone")

	ino = $(ino --lookup ${ino} "bullseye")



	griduserhost commands to griddb:

	ino = griddb:///rootstr get "file://bangfish.local"

	if ( ino == 0 )
	{
		dcel = griduser:hiena source "file://bangfish.local"

		dno  = griduser_next_dno( griduserhandle );

		err  = griddb:dcel put %{dno} %{dcel}

		dcel = griddb:dcel get %{dno}

		ino = griddb:///axpa new
		er  = griddb:///axpa set %{ino} dcel %{dno}
		er  = griddb:///rootstr set "snafu://bangfish.local" %{ino}
	}
	
	ino = griddb:///axpa lookup %{ino} "home"
	ino = griddb:///axpa lookup %{ino} "redboy"
	ino = griddb:///axpa lookup %{ino} "project"

	/* preserve local files under mountpoint before mounting */
	dno = griddb:///axpa get %{ino} dcel
	svr = griddb:///dcel get %{dno} server
	svr = griddb:///server init preserve_local_files %{dno} %{svr}
	er  = griddb:///dcel set %{dno} server %{svr}

	/* restore via: */
	svr = griddb:///server restore preserve_local_files %{dno}
	er  = griddb:///dcel set %{dno} server %{svr}

	/* more lookups */
	ino = griddb:///axpa lookup %{ino} "music"


MORE OLD WORK IN PROGRESS ...

Consider a file scheme URI with authority and host.

	file://<user>@<host>/<absolutepath>

The following two URI's have the same access path.

	file://blueboy@bangfish.local/home/redboy/theproject
	file://redboy@bangfish.local/home/redboy/theproject

An access path can be identified by a file scheme URI **without** the authority.

	file://bangfish.local/home/redboy/theproject

The access path database is implemented by a directory tree structure.  Finding an access path object is performed via a pathname lookup.

During a mount proceedure, a pathname is submitted for lookup to locate the access path object of the mountpoint if one exists.

	mount file://bangfish.local/home/redboy/theproject

	apathdb lookup bangfish.local /

	error: no such record

At which point we can create a tree.

	apathdb create bangfish.local /
	apathdb create bangfish.local /home
	apathdb create bangfish.local /home/redboy
	apathdb create bangfish.local /home/redboy/theproject

At each point of this tree we should load the cosm.

	apathdb create bangfish.local /
	cosmos load cosm /




Lets test some hypothetical trains of thought through psuedo cli:

	gridget uri file://blueboy@bangfish.local/home/redboy/theproject axno

	error: no such record

	gridget axpa file://blueboy@bangfish.local/home/redboy/theproject

	gridmk file://bangfish.local/
	gridmk file://bangfish.local/home
	gridmk file://bangfish.local/home/redboy
	gridmk file://bangfish.local/home/redboy/theproject

	gridmk uri file://blueboy@bangfish.local/home/redboy/theproject \
		axno 20001

	gridget uri file://redboy@bangfish.local/home/redboy/theproject axno

	20001

	gridmv uri file://blueboy@bangfish.local/home/redboy/theproject \
		file://blueboy@bangfish.local/home/blueboy/theproject

	gridget uri file://redboy@bangfish.local/home/redboy/theproject axno

	error: no such record

	gridget axno 20001 uri

	file://bangfish.local/home/blueboy/theproject

	gridget axno 20001 parent

	error: no such record

	gridmk axno 20002
	gridmk uri file://bangfish.local/home/blueboy axno 20002
	gridset axno 20001 parent 20002

	gridmk uri file://bangfish.local/home/netherworld axno 20003
	gridset axno 20001 parent 20003

	gridget axno 20001 uri

	file://bangfish.local/home/blueboy/theproject

	gridmv uri file://bangfish.local/home/blueboy/theproject \
		file://bangfish.local/home/netherworld/theproject




\section  command_parsing  Command Parsing

The command is parsed into these parts:

	gridmsg := uri ' '+ cmd ' '+ args ;

	uri := SCHEME ':' userhostarea? contextpath? ;

	userhostarea := "//" userat? hostandport ;

	userat := USERNAME '@' ;

	hostandport := HOSTNAME ( ':' PORTNO ) ;

	contextpath  := FILEPATH ;

	cmd := FILEPATH ;

	args := CLIARGS ;


From this we can determine the context using the scheme, host and filepath.

The `gridengine` uses the context to route the messages:

	if scheme == "griddb"
		submit to griddb broker

	if scheme == "snafu"
		submit to deligation broker

	all other schemes, ie. "file", "ssh", "http", "ftp"
		submit to server module broker



\subsection  griddb_broker  The Grid Database Broker

The Griddb Broker translates the message into a `griddbmsg` and sends the message over the `griddbsocket`.



\subsection  deligation_broker  The Deligation Broker

The deligation broker translates the message into a `deligation frame`.  This is described in [Grid Instruction Engine](@ref gridmsg_engine).

Each member of the [Grid Topology](@ref gridtopology) gets an entry in the Deligation Frame.

When the Deligation Frame is complete, the broker goes down the list and submits each entry's command to that entry's Grid Topology member via a Grid Guest.  The only member which does not get a Grid Guest is the localhost.

The localhost's deligation entry is sent to Local Command Execution.


\subsection  source_broker  The Server Module Broker

The server module broker manages the server modules.  There is a basic set of file commands which all server modules use.  There are additional commands which some modules may use but not others.

Basic Server Module Commands:

	open
		returns a file descriptor

	close
		releases the file descriptor

	rm
		sends remove instruction to server module

	...
		(all the usual file commands)

Special Server Module Commands:

	proxy PROXYSERVER
		initializes a server module named by PROXYSERVER with the uri,
		swaps out the original server module for the proxy

	unproxy
		swaps back in the original server module, removes the proxy





/* older draft */

The first bit of the command:

	context

Should call up an Cosmos Cascade Context Frame aka Access Frame.

	aframe

The aframe has a collection of commands in it's Cosm.

	aframe/
		cosm/
			bin/
				%{cmd} --> %{external_ref}
				%{cmd} --> %{builtin_ref}
				%{cmd} --> ...
			usr/bin/
				%{cmd}
			usr/local/bin/
				%{cmd}
		env/
			PATH="/usr/local/bin:/usr/bin:/bin"
		fs/
			.cosm/	--> aframe/cosm
			%{node}
		...


The commands may be builtin.  Or they may be external.

The context is broken into interesting pieces:

	context := scheme "://" [[ auth "@" ] host ] "/" contextpath

*First draft protocol* omits the scheme auth and host and identifies a context merely by the contextpath.

	context := contextpath

The context allows a message to target a particular host.  If the host is left out, it implies any/all hosts may execute the command.

Therefor, it is expected that the `contextpath` component of the message context should be syncronized among hosts.  For this purpose, a Grid-Globally Unique Context Identifier looks like:

	grid_uniq_context_id := scheme ":///" contextpath

We can have a database of 'griddb contexts', each record identified by a `grid_uniq_context_id`.

The record data should be `aframe` types.  Each aframe has a `dcel` and the `dcel` contains the Cosmos Cascade structure as outlined earlier.  As such, the regular lookup mechanism can be used on the aframe to locate a command.

	lookup_at_aframe( aframe, cmdpath )

The result is a dcel identifier `dno` which references the `dcel`.

A command is therefore represented by a dcel.

**Running a Command**

The command runs "inside" the access frame.

	griddb:///rootstr set 'snafu://bangfish.local' 12

The aframe might look like this:

	aframe/
		cosm/
			bin/
				"get"
				"set"
		fs/
			.cosm/
			'snafu://bangfish.local'
				12

(note: we can allow "/" to appear in file names if we need -- as this is not just a filename but a "key" of a kvpair.)

We can implement these commands as builtins if the generic command template is a variable argument function.

	(void *)(*generic_builtin)( struct aframe *a, ... );

	void *get( struct aframe *a, ... ) {
		va_list va;
		...
	}
	void *set( struct aframe *a, ... ) {
		va_list va;
		...
	}

So, because these are builtin's -- AND NOT DYNAMICALLY LOADED.  We don't have to sandbox them -- if we trust ourselves.  We just pass the aframe structure as the first argument.  And then use the aframe and dcel API's to manipulate the aframe and structure.

Dynamically loaded commands are a possibility.  But, why not just use external commands?

**Command Routing between User and Multiuser**
(see "Setting Up the Aframes-based Griddb" in the "Griddb" document)

The Griddb can be implemented as aframes with built-in commands.  But because the Griddb is multi-user and must run as a trusted user, it cannot run external nor dynamically loaded commands.

The Griddb serves multi-user metadata to local Griduser Daemon clients.

The Griduser Daemon reads gridmsg's from Snafu FUSE Volumes and from other Griduser Daemon's that have ssh'd onto the host.

The Griduser Daemon re-writes the gridmsgs to dispatch on the grid via ssh tunnels to other Griduser Daemon's on other hosts.

The Griduser Daemon loads aframes, and runs commands inside the aframes.

The Griduser Daemon must load and run the dynamic modules, such as the lookup module or the source server modules -- including the all important `preserve_local_files` server. 

Let's examine the "playground" from earlier.

	ino = griddb:///rootstr get "snafu:://bangfish.local"

A multiuser request ok to serve as long as ACL's are followed.

	ino = griddb:///axpa lookup %{ino} "home"
	ino = griddb:///axpa lookup %{ino} "redboy"
	ino = griddb:///axpa lookup %{ino} "project"

The above would rely on the "lookup" module which needs to run inside User Space.

Could the code be run inside User Space?  Here's the client message:

	griddb:///rootstr get "snafu:://bangfish.local"

Griduser sends a message to Griddb:

	to griddb: get aframe "rootstr"

Griddb sends aframe to Griduser

	rootstr/
		cosm/
			bin/
				"get" --> Griduser:funcs/get
				"set" --> Griduser:funcs/set
		fs --> Griddb:dcel/%{dno}

Think of it this way: the Griddb stores databases of primatives.  The Griduser Daemons request, submit and remove primatives from Griddb.  Each Griduser Daemon keeps its own database of builtins, its own database of modules, its own database of module data.  Any id's from the Griduser databases can be stored as values in the Griddb primatives.  So structures made from Griddb primatives can include items from the Griduser databases.  This is especially useful if the Griduser databases are cannonical among different users and on different hosts.

	Griddb daemon/
		griddb primatives databases
		primative storage logic only

	Griduser daemon/
		griduser databases
		control logic
		loadable modules
		external commands
		ssh network

	Snafu FUSE Volume
		gridmsg interface to Griduser daemon
		VFS representation of Griduser file space


**External Commands**

It's a bit more involved to get external commands going, but it may well be worth it.

In the case of external commands, the command entry should have a pathname to a command executible.

	/usr/local/griddb/bin/get
	/usr/local/griddb/bin/set

Before executing the command, we have to do a couple things.
- Export the aframe as a Virtual file system
- Create a Sandbox

Head spinning yet?

We could create a private FUSE mount inside a safe place and the root level of the filespace contains dynamic aframe directories by a hash of aframe_id, process id and thread id.

	aframes_fuse/
		%{aframe-process-thread-id}/
			cosm/
				bin/
					get
					set
			env/
			fs/
				.cosm
				%{key}
					%{val}
				%{entry}
					%{val}
				%{entry}
					%{val}
			vroot/
				bin/
					get
					set
				%{process_thread_working_dir}
				...
		%{aframe_id}/
			...

(Also, under both these models, maybe there's no reason why the 'fs' directory needs to be an actual directory.  It could be a database.  As long as 'get' and 'set' are able to access it?)

The sandbox could be created by running an LXC over a `vroot` directory.  And the `pwd` could be sandboxed to `process_thread_working_dir` by forcing all lookups outside of its filespace to return to it.

**Recap**

For the first draft implementation I don't have to implement external commands.
