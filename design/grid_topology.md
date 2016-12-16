Grid Topology {#gridtopology}
=============

Grid topology is above the level of the file hierarchy.  It specifically deals with which grid hosts are within range.  However, not all grid hosts will be mapped to the same data, nor have the same permissions for that data, so in another sense, the grid Topology has to be allowed to be reconfigured at each level of the file path hierarchy.

A Grid Topology Object keeps the grid members.

The idea is that not every member of the grid needs to know about every other member.

Grid members basically act like redundant routers to other members.  So the route can change, the members can change.

But ultimately, the grid members need to have some form of stable addresses so they can relay messages to those members who might have moved.  IP Addresses.

The Grid Topology is kept in the `.cosm` metadata for each directory.  The directory behaves Copy on Write.  So the same Grid Topology object will be reused by reference by a majority of subdirectories.  When a Subdirectory requests the Grid Topology be recalculated, it will recalculate for all inheritors of that Grid Topology.

If the grid memberships is reconfigured within the `.cosm`, then the Grid Topology will have to be rebuilt from seed as prescribed in the new configuration.


\section bypasstopo	Topology Bypass

For simple networks, and for Proof of Concept.  The complex topology generation can be bypassed.  Having Seed Members directly become the "Inner Circle" (see below.)

The basic calls to Generate Topology would still be deployed when syncing the `.cosm` of any direcotory or aframe.  But the complex calculations will be skipped.  The calls will remain as vestigial stumps for later development.


\section grid_config	Grid Config

The configuration is held inside the `_cosm` folder.


\section generatetopo	Generate Topology


Generate Topology () makes a Grid Topology Object.  This object is only a snapshot.  Each time it is called, the topology may change.

**SEEDING**

The source would be Local Grid ID, and Expression of Grid ID's or LIST of Grid ID's.  This allows a MASSIVE membership of like a BILLION gridhosts to be configured to run together, without requiring any one gridhost to configure all.

If the host ID's are expressed as IP addresses or some computable id, we can just use a computation to select a group of IP addresses to be polled.  If each gridhost polls a cluster of ID's that are close to its own.

And starts doing a little dance of mapping latency.

**LATENCY SHUFFLING**

A gridhost maps the latency of the hosts in its cluster.  It shares that info with each host.

When it gets latency info from another host, there may be other hosts on that latency mapping.  Then it begins trading trading latencies with other hosts.  Longer for Shorter.

So each host tries to trade away the longest latencies in its cluster.  Thus shuffling the routes into a shortest route paradigm.

**TRIANGULATION**

As latency in the cluster gets to a happy place.  Each host begins triangulating with its neighbors to map directionality.  The target state of the Grid Topology is a fully triangulated cluster around the local Gridhost.

**ROUTING**

Out of the Cluster, a Gridhost selects its Inner Circle.  These will be the brodcast targets for its Instructions.

A Grid Route can be built per Instruction Origination.

A grid instruction on its Originating Gridhost hashes to a UUID.  The Current Host Submits the Origin UUID to each host in its Inner Circle.  If a host already has the UUID in its processing queue, it will reject the submission.  The Originating Gridhost marks each rejecting host off the list for this instruction until it arrives at the Chosen Few.

The Chosen Few is the final model within the Grid Topology Object that is used for propogating the Grid Instruction.  Along with the Grid Instruction, it could also send a Topology Hint, showing which hosts have already rejected the instruction.  This would save the next host time without polling those previous hosts.
