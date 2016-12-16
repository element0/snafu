Grid Instruction Engine {#gridmsg_engine}
=======================

The Grid Instruction Engine processes incoming [grid messages][gm] and creates a [Deligation Frame](@ref delegframe) with component grid messages which it distributes.  It recombines the results.

[gm]:@ref gridmessage

The return value is sent back to the submitting node which may ripple back across the grid.


\section delegframe	Delegation Frame

The deligation frame is created with one "slot" for each available Gridhost from the Grid Topology.  (see Grid Topology)

The slot fields are as follows:

	gridhostid, access_frame_ID, commandline, return_frame

The incoming instruction is basically:

	access_frame_ID, commandline

ie.

	20012, "munge file[1-20].txt"

The local Gridhost examines the commandline and, if possible, componentizes it into one instruction per Deligation Slot, including Self.

	Self,  20012, "munge file[1-5].txt",   <empty>
	Host2, 20012, "munge file[6-10].txt",  <empty>
	Host3, 20012, "munge file[11-15].txt", <empty>
	Host4, 20012, "munge file[16-20].txt", <empty>

It sends instructions to the other hosts and fills the return_frame column with their return values.  When all hosts have returned, it combines the values per the "munge" command's grid specs.

ie.

	munge_merge return_frame[1-4]


\section walkoverview	Overview Walkthrough

(These Examples use Shell Commands as a demonstration)

The Grid Instruction Engine receives a Grid Instruction.

	read access_frame_ID, commandline

The command line is broken out

Updates local version of access frame by requesting aframe update data from sender as needed.  (rsync used as an example idea of updating just the bits that need updating).

	rsync $sender_aframe $local_aframe

Clones $local_aframe for COW.

	cp --reflink=auto $local_aframe $return_aframe

Makes sandbox from access frame (uses cached sandbox if available.)

	gridengine_lxc_conf_from_aframe $return_aframe $lxc_conf
	
Runs Command in Sandbox (name sure all outputs get copied to some known location before the LXC is broken down)

	lxc-execute -n $tempname -f $lxc_conf $commandline

Generates COW Copy on Write access frame as modified by command.  (use the known output locations to update the return_aframe.)

	gridengine_modify_aframe $return_aframe

Returns Copy on Write updates of access frame.

	rsync $return_aframe $sender_aframe

Merges COW with local aframe.

	mv $return_aframe $local_aframe


delegates to a Grid Topology


\section gridinstrinterface	Grid Instruction Interface

A grid instruction comes from an Instruction Origin or an Instruction Deligate.  An instruction Origin might be the any function serving a Snafu mount on a particular host.  The Origin must format the instruction as "aframe" and "command".

For Origin, the Gridhost sets up a 

A grid instruction is passed to a grid host in a **Gridhost Protocol Message  stream**.

The context of the instruction is a `cosmos_aframe`.

A Grid Instruction Message

	cosmos_aframe_id, command_name, [args]...

If the Gridhost does not match `cosmos_aframe_no` in its aframe database, it asks the sending Gridhost to provide it.  A Grid Instruction is used to ask the sender.


	cosmos_aframe_id=NULL, command_name="get",
		arg[1]="aframe", arg[2]="cosmos_aframe_id"


\section processing	Instruction Processing

The Instruction Engine loads the `aframe` and sets up a "shell environment" for the grid instruction.  Then runs the grid instruction as a command inside that environment.  The `aframe` environment provides stdio and virtual filespace.  So the grid command can write to stdout, err, read from stdin, use envars.  Write to files.

The command name is first matched for a builtin.

Builtin commands are designed to take an `aframe` as an argument and work within the `aframe` API's sandbox.

External commands are run inside a sandbox, created by the `aframe`.

As the command runs, it modifies the `aframe`.  Copy on Write modifications.  Only the modifications are returned to the sender.




\section gridinst	Grid Instruction

A **Grid Instruction** can be visualized as a command line.

	command [opt ... ] [ arg ... ]

Where `command` is either an executable, script or builtin available on one or more gridhosts. (see [Gridhost Command Availablility](#cmdavail))

If the `args` are expressed as a Grid Argument Expression the expression can be dissected into component expressions.  Each component expands to a subdomain of the GAE.  All components subdomains expanded will equal the GAE expanded.


**Aggregate Commands**

An aggregate command can be used reursively on its own output, in any order, and achieve correct results.

	add 1 2 3 4 5

Can be broken into 

	add $(add 1 2) $(add 3 4) 5	

Each subshell could be operated on by a different gridhost.


**Single Shot Commands**

Some commands may only work on a single arg and aren't required to operate on their own output with correct result.

	munge onlyone

If we give it a RE as input

	munge [a-e]

The commandline can be expanded into multiple commands.  (Each subshell and background execution here can be processed by another grid host.)

	(munge a > outa) &
	(munge b > outb) &
	(munge c > outc) &
	(munge d > outd) &
	(munge e > oute) &

The result is a list `out[a-e]`.


**Serial Processing Commands**

Some commands may require serial processing

	divide 100 5 2 2

becomes

	divide $(devide $(devide 100 5) 2) 2


Above, each subshell would be executed by another grid host, but 
because each shell must wait for its subshell to finish, this serialized recursion is not efficient.  This type can't be run in parallel.


**Sequenced Data, Parallel Processing**

Some data sets may be sequenced.

	grep [a-z]+ data.txt

This could be run in parallel

	(grep [a-m]+ data.txt > out1.txt) &
	(grep [n-t]+ data.txt > out2.txt) &
	(grep [u-z]+ data.txt > out3.txt) &
	cat out[1-3].txt

and

	(ls [a-m]+ > out1.txt ) &
	(ls [n-t]+ > out2.txt ) &
	(ls [u-z]+ > out3.txt ) &
	cat out[1-3].txt

and

	(cat out1.txt > out11.txt ) &
	(cat out2.txt > out12.txt ) &
	(cat out3.txt > out13.txt ) &
	cat out1[1-3].txt

**Difficult to Parallelize**

	diff file1 file2

Because diff only works on two files at a time.  And is not designed to input its own output in a meaninful way.


**Conclusion:**

Predictibly Aggregate Commands can be used as a grid instruction.

Other commands would need to be broken down into scripts containing Aggregate Commands among other locally executable commands.


\subsection cmdtypes Command Type Configuration

Each command can have its type configured in an information database.






\section prodinst Production Instructions

Hiena Production Instructions can be run via the Grid Instruction Engine.


\section gridtopo Gridhost Topology

Each Gridhost builds its own Topology object, which essentially maps its "Cell" of the Grid Network.

Multiple Topologies can exist per Gridhost.  A Topology is relative to a Volume, Subvolume or Subdirectory Tree.


\section cmdavail Gridhost Command Availability

Each Gridhost may be independently configured.  The ideal is for each gridhost to be configured identically.  But that may not happen in practice.  Some machines may have a different set of binaries and scripts and if the Gridhost library has a different version, there may be different builtins available.

When a Gridhost builds its Topology
