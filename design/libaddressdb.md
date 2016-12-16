Address Database Lib {#addressdb}
====================

[TOC]

Library creates databases of URL's, URL Bundles and Address Translation Tables as well as an Address Object.

It is used by Gridhost and Server Module.

The Gridhost uses this library as part of the grid library suite which includes the hiena databases.  The entire suite is meant to be run under the same 'gridhostd' proccess either user-space (single user) or root (multi-user).

The Server Module only needs to use the address database library.  Doesn't use the entire grid suite.  And can run either in user-space or multi-user independent of whether the gridhost is in either space.


**Dependencies**

Gridhost will use the address database to keep the sources for all domain cells.

A server module will use the address database to keep a translation table between the addresses that Gridhost uses and the internal addresses that it keeps.

**Address Translation Architecture**

Gridhosts share a database of addresses, one address per file fragment (as parsed by hiena).

On each host, per server module, there is an address translation table.  The server uses these to transform the grid global fragment address to a server module address.


\section walkthru	Walkthrough: Journey of an Address When Mounting a Volume


On my laptop,

	snafu test/snafuthis

Gridclient takes the `test/snafuthis` relative filepath and turns it into a URL.  `grid_filepath_to_URL()` (This URL translation is not part of the addressdb library.  The Gridclient doesn't make any addressdb calls.)

	file://raygan@bangfish.local/home/raygan/test/snafuthis

\subsection makegridaddr	Add URL to Grid

Gridhostd takes this URL to `gridaddr_addurl()` which checks it against the URL string database and if no matches, makes it into a `grid_addr`: 

	scheme: file
	user:	raygan
	host:	bangfish.local
	filepath:  /home/raygan/test/snafuthis
	fragment:  ""

The `grid_addr` is analyzed and matched to a `server_module`.
	
	server_module: file

\subsection preservelocal	Preserve Local Files Before Mounting Over

All `"file://"` scheme URL's are treated as local to the Gridhost.  Which uses a "file" `server_module` to access the local file system.

Snafu submitted this URL to the grid for the purpose of including the contents at the URL in the root of the file system.  However, the Snafu file system allows for bind-mounting several sources.  After returning from the submission call, Snafu will bind its volume over the URL location.  During the submission call, Gridhost requires that the "file" `server_module` preserve access to the pre-bound contents of the mountpoint.

Gridhostd calls the `server_module` `->` `server_mountaddr()` with the `address record`.  The `file server_module` locates the data under the local address and preserves a link to it.

The Proof of Concept implementation of `file.server_module` mounts the pre-snafu filepath to a temporary location.  Something like:

	filepath:  /tmp/raygan/snafu/snafusource001

And creates a mapping between the filepaths and assigns an internal number to the address.

	server_addr:
		filepath:  /home/raygan/test/snafuthis
		mapto:     /tmp/raygan/snafu/snafusource001
		server_addrno:	   10106

Gridhostd adds the server module's address number to the `grid_addr`.

	url:    file://raygan@bangfish.local/home/raygan/test/snafuthis
	scheme: file
	user:	raygan
	host:	bangfish.local
	filepath:  /home/raygan/test/snafuthis
	fragment:  ""
	server_module:  file
	server_addrno:	10106


\subsection bundleid	Assign Bundle ID

Now the next important bit, the Bundle ID.  (This step comes after Preserve Local Files b/c if Preserve step fails, we won't add the URL to the DB.)

If `gridaddr_addurl()` found a match to the url in the first place, it would just retrieve the record.  If the URL was bound with other URL's the URL will have a Bundle ID.  (We'll explore more when we get to how and when URL's are bound.)  For now, because this is a fresh address we assign it a fresh `bundleid`.  And a fresh `grid_addrno`:

	bundleid: 00001
	grid_addrno: 20001

We create a corresponding Bundle in the Bundle DB.

	bundleid: 00001
	grid_addr_members:
		- 20001


\subsection aframeno	Assign Access Frame

Gridclient sent a `cosmos_grid` object with the URL it sent.  The `cosmos_grid` object must have been initted by `grid_open()`.  This would have set up (`cosmos_init_aframe()`) a Start Access Frame `cosmos_aframe` in the `cosmos_grid` object.

	aframe:
		user: raygan
		host: bangfish.local
		root: /
		grammarurl: file://bangfish.local/usr/local/lib/cosmos/types
		grammar: <grammar object>
		auth: <user auth object>
		env:  <environment object>
		aframeno: 30001

The `cosmos_aframe` gets an access frame is stored in the Access Frame DB.


\subsection prodinstr	Create Domain Cell

Gridhostd creates a **Production Instruction** (`grid_instruction`) from the `grid_addr`.

	grid_instr_id: 50001
	commandline: hiena_sourcerer -grid_addr 20001
	commandpath: ""
	builtin: hiena_sourcerer()
	argc: 3
	argv:
		- hiena_sourcerer
		- "-grid_addr"
		- 20001

Gridhostd passes the grid instruction to the **Grid Instruction Engine** which uses the current Grid Topology to break down the instruction and process it most efficiently.

On each gridhost the broken down Production Instruction is processed by a function referenced by the instruction's `argv[0]` or `builtin` and the result is ultimately returned to the originating host and recombined.  In our current situation the Gridhostd might be the only host in the topology.

The result of this Production Instruction is a **Domain Cell** (`hiena_dcel`) which goes into the **Domain Cell DB**.

	dcel:
	  dcelno: 40001
	  directory: ""
	  map: ""
	  fragment:
	    role: container
	    contents:
	      - role: content
	        source: <grid_addr reference>
	  production_instruction: <production instruction reference>

(This result is so stupidly simple, you're probably wondering why we would possibly need a grid computer to calculate that.  The Grid Instruction Engine might be smart enough to catch this and *not* deligate.)

The content fragment of the domain cell uses the `grid_addr` as its fragment address.

The Domain Cell is added to the Access Frame.

	dcel: 40001

\subsection configcosm	Configure "Cosmos"

The "Cosmos" is a Cascading Configuration of:

	Users
	Tools
	Object Types
	Devices
	Sync
	Settings

These elements can be inherited, redefined or white'd out for each path segment in the Snafu volume's hierarchy.

The settings can be accessed through a `.cosm/` or `_cosm/` subdirectory available from any directory of the volume.

The configuration itself can be configured.  The above is my opinion of what I observe a non-technical user needs access to.  If you want, you can set yours up for:

	bin
	dev
	etc
	home
	lib
	sbin
	tmp
	usr
	var

The `.cosm` meta directory is an artifact of the file system itself and automatically exists, automatically inherits its parent directory's configuration -- but doesn't automatically get written to permanent storage except when explicitly written to by the user through the file system.  And then, only the configurations which the user write's to that path will be written, not the inherited ones.  It carries the same ACL capability as a normal directory, so you can control user access.

The `cosmos_aframe` is passed to the **Cosmos Cascade Function** the Cascade Function uses a **Dcel Reader Function** to read the Dcel, which incites the **Hiena Mapping Engine** to map the dcel as needed.  The Reader Function allows the Cosmos Cascade Function to navigate the Dcel's byte stream, map and directory stream contents without knowing how to communicate with its sources.

The Cosmos Cascade Function reads the Dcel's directory and branches for configuration patterns and parses those branches into the various places.  Right now I'm concerned with the following:

	.cosm/Types/*	{ cosmos_load_types_to_gridgrammardb(); }

	.cosm/Sync/*    { 
			  cosmos_load_sync_urls_to_addressdb();
			  cosmos_load_sync_rules_to_production_instruction();
		  	}


Of particular interest right now is how Cosmos Cascade Function looks inside the configuration and extracts more source URL's to the Address DB.

`cosmos_load_sync_urls_to_addressdb()`

Also, the "Sync" configuration is used to create a Production Instruction which may bind the original address with other local or remote urls.

`cosmos_load_sync_rules_to_production_instruction()`

The result is a new Dcel.  Which would, through its Production Instruction Record, have an internal reference to the original Dcel.

Also the Cosmos Cascade Function will cascade the .cosm configuration onto the configuration of the current `cosmos_aframe` and will result in a new, _cascaded_ `cosmos_aframe`.

\subsection volumeroot	Volume Root

The new Dcel will be linked into the new Aframe.  And the new Aframe will be saved in the aframe database under a new number.  When mounting a volume as we are now doing, this aframe becomes the volume root.  It's ID is returned to snafu as the root ino.




\section lookupproceedure	The Lookup Proceedure 

Snafu's ino's identify a `cosmos_aframe`.  Snafu simply passes a ino with a string to the Gridclient lookup function.

Gridclient lookup function passes a Gridclient Protocol Message to Gridhostd.

Gridhostd translates ino to aframe.  Passes aframe and string to Lookup Module.

Gridhost checks aframes Access Path Branches for string match.

No match:
- Lookup Module translates lookup string into Production Instruction.
- Grid Instruction Engine processes Production Instruction into Dcel.
- Cosmos Cascade creates new cosmos_aframe around Dcel
- put new cosmos_aframe in Access Path Branches
- return new cosmos_aframe

Match:
- string match references a descendent aframe
- Synchronize Aframe (includes syncing dcel via its Production Insruction)
- return descendent aframe





\section multiuserchallenge	Multi-User Binding and ACL's

**Multi-User Challenges**

Either way, if Raygan gives Nathan permission to A, not to B.  Raygan binds A to B.  Nathan writes to A.  The system writes A to B, then Raygan has effectively given Nathan write permission to B.  ACL's must be bound as well as URL's.


**Incoming URL**

URL string, ACL, User Authority, Action

Nathan must be a user on my computer.  Through snafu, his gridhost ssh's into my machine as Nathan.  Then connect via a Unix Socket to my gridhost -- running as Raygan.  Raygan Gridhost can access Raygan's files, but can't access Nathan's files.


## Case 2 ##

So if Nathan adds URL A.  He passes his Auth, "add to grid" Action and URL.
The Grid must approve his Auth, Action and URL through a Grid-Level ACL.
The Grid looks up the URL in the database
	must approve his Auth, Action on the URL-level ACL.
The Grid asks the server of the URL for it's ACL.
The server of the URL must test his Auth, should provide an ACL for the URL. 

   Raygan adds URL A and B

**URL ACL Binding**

Absolute URL's which are bound together will all share the same set of ACL's.

If Raygan and Nathan share A.  Nathan only has permission for A but does not have permission for B.  But Raygan has permission for both.  Raygan binds A and B -- because he has permission for both.  If B may be updated with data from A.  And A may be updated with data from B.  Thereby giving Nathan access or influence over B.

Granular ACL.

If Nathan has read perm on B, but not write.  Anything Raygan writes to A+B is going to be readable by Nathan on B.  But lets say the write operation itself has Raygan's ACL.  If the host of B has fine grained ACL's, raygan can still multiplex his data to A and B, but withold read access from subcomponents of B.  If B does not have fine grained ACLs.  Then Raygan must adopt B's ACL's for a section of his data, or must exclude B from the multiplex.
