
Snafu	 	{#mainpage}
=====


Snafu is a file system which:

- binds existing local and remote sources into one distributed file system
- creates file paths into data structures inside any file

The sources are represented by **addresses**.

The addresses are accessed via **server modules**.

The server modules connect to the actual server of the address, remote or local.  The server modules implement a single API for the snafu internals to use.

Snafu internals are implemented as a **Grid Computer**.

A single Gridhost Daemon runs in userspace as long as there are open connections on the **Grid Socket**.
It is triggered by Snafu or run stand alone.

The userspace gridhost connects to a remote gridhost when an address specifies.

The **connection is secure.**


## Inos and Access Paths ##

A Snafu volume is made of Access Paths.  Access Paths are represented by Access Frames.  The ino's of a Snafu volume represent Access Frames.

The ino number system of an individual Snafu volume and the Access Frame identities within the grid file system are decoupled.  Snafu volumes with identical content may have different inos for the same access frame.

Also, the file system used on the grid decouples Access Paths from Data Domain.  That is multiple separate Access Paths may point to a single Domain of Data represented by a single Dcel.  On the grid, Dcel identities are distinct among Dcel's.  Access Path identities are distinct among access paths.

At this time there is no requirement that Access Path's or Dcel's on the grid have a numerical ID.  There is a requirement that the identities be unique, shareable and collision resistant.




## Construction Plan ##

### Libaddressdb ###

Gridhost and Server Modules both use [Libaddressdb](@ref libaddressdb) to manage their address databases.  Gridhost uses libaddress to format its address database.  Server Modules use libaddress to manage address translation tables.

Your job is to build the library that handles storage, association, translation and retrieval of addresses.




<div id="hiena"></div>
### Hiena ###

Hiena is a Hierarchy Analyzer.

Hiena reads a domain of input and builds a map.

A map has two kinds of significant mappings:  [**Fragment Cells**] map physical areas from the stream and [**Map Cells**] identify objects within the fragments. 

[fragment cells]:
[map cells]:

Within those types, there are two subtypes, **Abstract map cells** carry metadata about a fragment or object but have no physical location.  And **Container Fragment Cells** contain other fragment cells.

Hiena's domain of input includes a **domain stream** and any existing map.  

The domain of input is recorded as a **Domain Cell**.

The domain cell is a product of a Production Instruction which produces a domain stream and-or a map. (see Production Instruction)

To analyze the domain cell, Hiena relies on a cascading set of Grammars.  The Grammar Rules are implemented as bytecode Finite State Machines.  FSM's reference each other by rulename.  Grammar sets can be cascaded onto each other, effectively remapping the references and creating new grammars.


### Hiena Mapping Engine ###

The Mapping Engine acts on a Grammer Rule bytecode FSM.  The bytecode contains a series of rulenames and test conditions.

It first tests a rulename against the current position's map object, if found, it seeks the stream to the head of the next map object.  If it can't find a matching object, it scans bytes from the stream.

Scanning from the stream requires scanning from a Production Instruction.  In this way a Production Instruction behaves as sort of a micro-server for the domain of input.

The Mapping operation is executed slightly differently depending on the type of Production Instruction.


### Mapping Grid Instruction ###

The Mapping Engine is invoked from a Grid Instruction, as mapping can be broken down into parts in the following ways:

#### A: Prallel Sectional Mapping of a Single Source ####
Mapping is linear, starting at an origin and mapping away from that origin;  therfor points further from origin can only be interpreted in context from origin.  Therefor, an arbitrary byte sequence cannot be subdivided and scanned by separate eyes without first scanning the sequence for coherent sections.

But after scanning for sections, each section could be deligated to another host for scanning.  However, a shift-reduce parser will examine the smallest elements first, building to a larger whole.  Sectioning may be possible if we use a simple sectional grammar and then deligate a complex layer to the grid.  We also lose a lot of ground if we have to transfer the bytes to each other gridhost.  But, lets say there is a resident copy on each host.  We can use a simple parser on one host to identify sections, send the boundaries of a section to each node and the node would reply back with a map of the section.

#### B: Parallel Mapping of Multiple Sources ####
Lets say there are multiple sources - and a hash of the sources proves they are divergent.

We gain ground by having the nodes closest to the sources map those sources in parallel.

After mapping, the maps can be converged.  This can be done cooperatively creating "brackets" or "funnel" pattern among the cooperative hosts -- one host merges two maps, passes to the next host which merges two maps, etc.






### Production Instruction ###

There are four main types of Production Instructions.
- Source
- Bind
- Find
- Grind

Each type of Production Instruction is handled by a corresponding Producer Interface in the Hiena section of libgrid.

A Production Instruction references directly or indirectly the address of the source.

A Production Instruction is recorded in a Domain Cell.  It's influence extends over the creation of the Domain Cell and the creation of it's Fragments and the assignment of source Addresses to those fragments.  The Producer Interface function as a mini-server with read-write and mapping functions.


**Source Instruction** takes an address and generates a stream and/or a map and a dcel.  Source Producer Interface responds to a read request by providing a byte sequence or a map object sequence.  (Byte sequences correspond to files, map object sequences correspond to directories.  Either may be returned from the generic producer read() interface.)

**Bind Instruction** takes a set of addresses and generates a bundle of domain cells and a bundle of addresses (in the gridhost address db).
Each cell in the bundle is created via a Source Instruction with a single address.  The Bind Instruction generates a dcel to represent the bundle.
When the dcel needs to be mapped, the Bind Producer Function creates a grid request to map all cells in the bundle.  The gridhost breaks the request into smaller requests per the Grid Instruction Algo and sends them out to the grid for processing.
When the Grid Results return they should be compared  

Bind Instructions and Mapping.

The domain cell in question to be mapped would have a Bind Instruction.  The content fragment would have a source address assigned by the Bind Instruction.  

Each component of the Bind Instruction must be mapped and those maps converged before we can have a complete and acurate representational state.

Due to the nature of a unstable (by design) grid, we might not get to finish mapping each source.  These unfinished mappings would have to be journaled for completion at such later time when resources -- including network latency -- become free.  Therefor the mapping at hand, most likely from the localhost.  Would be the mapping displayed on that host.  After the maps are gathered, up to date and merged, the local mapping could be updated.  This would be perceived as a syncronization event.

To facilitate latent mapping, the Bind Production could maintain a separate map for each source as necessary and maintain it's own version of the domain cell as necessary.  Each source dcel would natively have its own map.  But additionally, the Bind Production could keep a "cannonical" version of each source dcel.  This version could consist of rearranged fragments.  It's the cannonical version that would be compared for the final merged mapping.

Native mappings --become--> cannonical mappings --become--> merged mapping

The merged dcel is the result dcel created by the bind.
The Bind Producer serves the byte stream and map data from the merged dcel.


**Find Instruction** performs a "select" like action on the contents of another dcel and produces a selection dcel.  The content fragments of the selection dcel are inherited and retained from the selected portions of the source dcel.  Thus no content is duplicated, it's contained in the new dcel by reference.

Reading and Mapping a Find Production Dcel

Reading and mapping are both fairly straightforward as the data is read from the source addresses defined by the fragments.  Map results are recorded in the same fragments, and thus become available to all relative containers of those fragments.  Thus if another process or host maps those fragments we can take advantage of that map.


**Grind Instruction** named thusly to rhyme with the proceeding two production types.  A "Grind" Instruction implements arbitrary expressions such as a math function, an encoder or a translator.  The result is a new domain cell which may or may not include fragments of the orginal or other dcels.  And may contain completely new *generated* material.

Expressions, Results and Data Integrity

The Grind Producer code manages how the material is cached.  Normal user expectation is that calculations are transient unless "saved".  However, Hiena generates paths to these calculations as if they were files.  So we may expect to delete a source and save the product.  There may be a way to define clear rules for this expectation and behaviour.  We should make sure we always maintain an expression of a domain stream unless it is explicitly deleted from the file system by the user.

As all Producer Interfaces are responsible for managing a dcel and its fragments, the Grind Instruction is responsible for assigning a source address to the dcel fragments.  It therefor has control over the address of the data result from the calculation.

The Grind Producer should assign a data address which is serverd by an intelligent cache server module which tracks retain counts of the cached data -- exactly like inode links.

Reading merely copies from the fragment.  Mapping maps the fragment like any other.  A Grind dcel can be used as input for any other type of dcel.


### Grind Instruction and Grid Instructions ###

Grind Instructions and Grid Instructions could be interchangable.  Both implement an arbitrary expression of inputs and operators.  Inputs on both can be domain cells.  Operators on both should be standard operators expected by any programming language.


### The Fudge Lookup Language ###

The Fudge Language is a Lookup Module.  It is designed to provide a compact programming language that can be run *inside* path segment names such that properties of a path segment can be assigned or queried during the path walk.  Complex queries akin to those performed by SQL can be performed at multiple path component levels.  And small expressions can be evaluated to generate a value or a byte stream at any path level.

The Fudge language was designed to work hand in hand with Hiena, as Hiena can generate a directory from a bytestream pattern.

### Lookup Module ###

**The Lookup Module API** provides methods built around the four basic hiena Production Instructions.  Source -- data linking; Bind -- data merging; Find -- queries; Grind -- expressions and translations.  Using the Find methods is more than necessary for mimicing a normal path "lookup" filesystem.

#### Derivatives ####

A derivative is a spontaneous conversion of a file from one format into another -- or a generative action.  It's exactly equivalent to running a Grind Production on a Dcel.


### FSM Grammars ###

You make the compiler for the FSM's.
But first you need to know the byte language of the FSM.
And there needs to be a FSM engine that takes the byte language and executes calls.
And there needs to be an FSM database.
And an FSM identifier translation table -- As one FSM might be supplanted by another.
Use the FSM Client API.

FSM System
- FSM mappable rule names
- FSM rule name maps
- Byte Language
- FSM structure
- FSM engine
- HBNF language
- Compiler
- FSM database

<div id="aframes"></div>
### ACCESS FRAMES ###

An **Access Frame** describes the *context* in which a domain cell is accessed.

There is an access frame for every segment in a file path.  The root access frame actually is not the filepath root, but the user environment *before* root.

The analogy is -- and stop me if you've heard this one -- a scientist, a preacher and a singer-songwriter enter an exhibition at a museum from three different entryways.  Each person begins with their own set of perspectives and beliefs about the topic: Pastimes of Rural Mongolia.  As each person makes their way through the exhibition their perspectives are modified by each showcase.  Each showcase cascades its perspective onto the perspective brought in by the user -- in as much as the user has capacity to interpret the contents of the showcase: ie, being able to read the placards.  And to each his own interpetation of what those words imply.  All three men arrive at a photograph of several locals watching a Falconry game from the sidelines.  The scientist sees the Falconry audience as an expression of the human mirror-neuons and capacity for vicarious experience.  The preacher sees the Falconry audience as lost souls looking for a quick feel good moment that will never satisfy the hole in their heart that only a relationship with God can fill.  The songwriter imagines that for each person in the audience, the Falcon symbolizes something: ability to the man with one leg, success to the young man and his bride, preservation of culture to the old couple, another good reason to have fun to the child.  Throughout the exibit, every showcase contributes context to the context carried forward from the previous showcase.

Every path segment contributes context to the context carried forward from the previous segment.  The root context begins with the user's device environment not the file system volume.

A shorter analog, the Aframe is a shell environment.  Subshells inherit from parent shells.

Aframe carries, among other things, the Grammar Rules used by the Hiena to scan data for hierarchy.  Some Grammar Rules should be predefined in the root context.  Other Grammar Rules can be defined at each path segment.

Aframe servers as the context structure for the gridhost language, the lookup module.




<div id="cosmos"></div>
### COSMOS ###

**Cosmos** the Cascading Operating System defines a data layout that allows layers of context to cascade gracefully from one path segment onto another.

The layout is a ".cosm/" directory under the directory to be modified.  Similar to an ".htaccess" file.  A ".cosm" directory can mimic a POSIX root directory structure.  .cosm/{bin,etc,lib,var,...}  This might be useful from a technical perspective.

I prefer to cascade a different structure:  .cosm/{tools,types,users...}

Somewhere in the cosm, Grammar FSM's are defined.  I keep my Grammar in "types" but you can keep it in "lib" if you want.

The Cosmos function uses a lookup() deligate callback to read from the file system.  The lookup deligate takes a "current context" as input to the Cosmos function.  Cosmos makes a copy of the "current context" modifies it by cascading the results from lookup.  And returns the new context.

For testing purposes the lookup function could be a direct accessor to an existing file system.  The "current context" could contain a current working directory pathstring.  Cosmos could perform the cascade without any dependencies on Hiena, Snafu, or Grid.


### Lookup and Cosmos ###

The **lookup API** loads/syncs/cascades the cosm for the current path segment at each lookup by making calls to Cosmos.

Lookup can function correctly with a single root context that doesn't cascade from segment to segment.  The root context would define the grammars needed for lookup to operate.  Every lookup would be performed with the same Grammar.

We can use this behaviour to *lookup* the ".cosm/" structure.  Then Cosmos can cascade that structure onto the previous context.

If we do this first during the path segment lookup *before* looking up the lookupstring we can modify the context that lookup will use for that string.

We implement this with a nested call structure:
- snafu calls lookup wrapper with Aframe no, lookupstring
- lookup wrapper calls Cosmos to load .cosm
- Cosmos calls actual lookup using current context and "cosm lookup expr" which evaluates to the entire ".cosm/.../..." branches.
- Cosmos cascades .cosm result onto current context, returns updated context
- lookup wrapper calls actual lookup using updated context and lookupstring
- lookup wrapper returns the resultant Aframe no.




### Libgrid ###

Libgrid is comprised of sections:
- Griddb Daemon
- Griduser Daemon
- Gridclient Library
- Gridsocket Library
- Hiena Library
- Cosmos Library

### Server Module Databases ###

- address translation tables

### Griddb Databases ###

(see [Griddb](@ref griddb))


### Gridsocket Communication ###

- unix socket
- Grid Client Protocol
- Grid Host Protocol

 
<div id="gridhostsec"></div>
### Gridhost ###

You need to build the gridhost daemon.  A stand alone daemon.  But it uses code from **libgrid**.  The stand alone is basically a wrapper.

The gridclient code from the libgrid will also spawn a gridhost daemon.  The **snafu** mounter will trigger this function by requesting a grid object from libgrid.

The **gridhostd** keeps retain counts of it's current clients.

The gridhostd calls the gridsocket code.

Gridhost translates incoming Grid Client Protocol on the socket and builds Grid Instructions.

Gridhost distributes the Grid Instructions to the other items in the grid.

Gridhost translates incoming Grid Instructions to local calls.

Your task is to create the gridhost code for libgrid and the standalone gridhost wrapper.

### Grid Instructions ###

Grid instructions are sent between hosts.
A Grid Instruction consists of a data selector and a procedure.
The procedure may be divisible.  The procedure is a byte code machine.

Gridhost receives a Grid Instruction.

Gridhost evaluates the difficulty.

Gridhost subdivides the instruction if necessary.

Gridhost distributes the divisions.

Gridhost acts on its own division.
It's at this point that Gridhost would call Hiena.

Gridhost collects the results.

Gridhost combines the results.

[Gridhost](#gridhostsec) returns results.

Grid Instructions are synonymous with Production Instructions.



### Gridhost Language API ###

Gridhost Language API retrieves accessframe and Working Aframe from **Aframe DB** by Aframe no's.
Modifies Working Aframe.

Some gridhost language calls translate to Grid Instruction.

Gridhost extracts address bundle information from accessframe.

Gridhost submits Grid Instruction to Bundle Hosts receives Grid Results.

Gridhost Language API applies Grid Results to Working Aframe.

Returns Working Aframe no.


### Grid Lookup Instruction ###

Gridclient API receives lookup as ( Aframe no, lookupstring ).

Gridclient sends Grid Client Protocol to Gridhost ( Aframe no, lookupstring )

Gridhost translates incoming Grid Client Protocol ( Aframe no, lookupstring )

Gridhost retrieves accessframe from **Aframe DB** by Aframe no.

Gridhost retrieves access URL from accessframe.

Gridhost retrieves Volumeid from Address DB by access URL.

Gridhost retrieves Lookup Module by Volumeid.

Gridhost calls Lookup Module ( Aframe no, lookupstring, callbacks )

Lookup Module receives call ( ... )

Lookup Module runs lookupstring through string Interpreter.

Lookup Interpreter through callbacks calls Gridhost Language API.

Gridhost Language API acts on accessframe, modifies new accessframe, returns Working Aframe no.

(Working Aframe persists across grid lookup instruction calls.)

Lookup Interpreter returns Working Aframe no.


### Grid Client Protocol ###

The gridclient protocol facilitates the FUSE operations.
[Grid Client Protocol](@ref gridclientprotocol)


### Gridsocket ###

Gridsocket code is used by both gridhost and gridclient to communicate through the same named socket on their host.  Libgrid holds this code for both to use.

### Gridclient ###

When the gridclient is called for a grid object, it trigger grid initialization if it hasn't been done already.  And then the gridclient spawns a **gridhostd** which will persist until all it's clients close.

The gridclient creates a connection object over the socket with Gridhost.

Gridhost and Gridclient share the same Gridsocket code via libgrid.

The gridclient API translates the API calls into **Grid Client Protocol**.

### Snafu Mount ###

Your task will be to build the snafu FUSE mount program.
Implement FUSE ops that use the libgrid **Gridclient** API.
Ask for a grid object -- this will trigger grid initialization.
Send a URL of the mountpoint to the grid object.
Implement a lookup function that sends a _lookupstring_ to the grid object
and gets a ino for the lookup.


## Architecture ##

(see [Architecture](@ref architecture))


## Development Feeds ##

Where one product feeds another...

![](../../design/devorder.svg)

### 2016 Implementation ###
- "snafu" mount utility uses FUSE

