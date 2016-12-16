Snafu FUSE Volume {#snafuvol}
=================


uses the following databases
- addrdb
- dceldb
- aframedb
- apathdb	"Access Path Database"


\subsection apathdb	Access Path Database

Access paths are how a user gets _to_ the data.  That may by obvious.  But Snafu separates data access from data structure.  However, access and structure can both be expressed with URI's.  But they are not analgous.

Access is the route the lookup mechanism takes as it acts on a URI.  Lookup this, then this, then this, then that.  Structure is a pattern of data: this contains this, which contains that.

	The house contains the living room.  But to get there, you might go through the back door, walk through the kitchen, walk through the dining room and then walk into the living room.  Or maybe you walk through the front door, walk through the foyer, and then into the living room.

Access paths are important to the Cosmos subsystem.  Cosmos creates a cascading operating system.  Each subdirectory of the file tree inherits the Cosmos configuration of its parent.  Complex systems can be inherited, including structure analysis grammars.  An access path builds context for the viewer.  If you walked through the foyer you might have seen pictures of grandchildren on the wall, and then met grandma in the dining room.  But if you came through the back door, you may have smelled the sweet dinner rolls she was cooking before you met her.  Both are two contexts on the same content.

In order to implement Cosmos on a mountpoint that might be several levels deep in the host filesystem, we have to track all the directories leading to the mountpoint.

Tracking pathway directories also allows us to identify if a new mountpoint is inside an existing mountpoint, we can see if a mountpoint changes locations inside the host file tree, and we can apply Cosmos cascading configuration in a predictable way from the host root directory outwards.

So in many ways, we need to design Snafu Grid to work on pathnames, doing lookups etc.  And then just use Snafu FUSE as an interface to get the host file system to wrap the Snafu Grid.

From that perspective lets define the snafu system as a service which takes a `command` and a `URI` and returns a result.  Lets say the system caches its meta data for future use.

And an accessory subsystem could be created that tracks the host file system for changes and then executes a Snafu command which will bring the snafu system up to date.



\section snafu_scope	Snafu File System Scope

The Snafu File system should have the capacity of tracking the entire host file space.

With each Snafu Suite call, whatever portion of the host filespace that is relavent to the call gets mapped into the Snafu Griddb databases.

The Snafu Griddb roots each filespace at each host.  ie. "snafu://bangfish.local/", "snafu://flyboy.local/", "snafu://redbaron.local/".

(As explained in section [Snafu URI Scheme]{#snafu_scheme}, a root filespace mapping needs to include the scheme name as well as the host name.)

(Hosts may have aliases which resolve to the same host.  The actual host name in the filespace mapping needs to first be normalized.)

A Snafu FUSE volume presents a branch of the Snafu File System as a mounted volume on the host file tree.


\section snafu_volume	Snafu FUSE Volume

Without any Snafu FUSE Volumes mounted, the use of `snafu` scheme URI's is straightforward: a URI is passed to a Snafu function; The Snafu Suite begins the pathwalk at the URI's pathroot; calls Cosmos to get the cascading Cosm configuration; reads the distributed storage ("flows") configuration; creates source dcel's for each storage location; binds the sources to one bundle dcel; reads the directory of the bundle dcel; looks up the next path segment from the directory; creates an access path node to the next segment; repeats until all segments found.

Each path segment may have more than one storage location.  Each segment's domain cell (dcel) records the source locations.  One or more source locations may be on the localhost's file system.  If there are no Snafu FUSE Volumes mounted, then the localhost's file system can be read from directly, enough said.

But if there are any Snafu FUSE Volume mountpoints on the localhost's file system then when the Snafu Suite tries reading from the file system thinking it is reading a local source, it may be reading the Snafu system.

When a Snafu FUSE Volume binds to a mountpoint, the Snafu system must preserve the underlying source for the mountpoint.

A simple idea: two different server modules could be used.  One, the local file server module which takes a pathname and reads directly from the localhost file system.  Two, the local preservation file server module which takes a pathname, performs an address translation on it to arrive at a protected address.

To set this up, the Snafu FUSE Volume calls the Snafu Suite to request a bind mount on a mountpoint.  The Snafu system resolves the mountpoint with it's normal pathwalk and then locates the dcel which references the localhost's file system.  Snafu will replace the dcel's server module with the preservation server module.  But first initializes the mountpoint with the preservation server.  The preservation server, with its own methods, preserves access to the underlying localhost's filetree at the mountpoint.  The preservation server keeps an address translation table between the logical address seen by the user and the private preservation address used by the preservation server.  Once preserved, Snafu replaces the dcel's server module with the preservation server module.  (The source address expression for the dcel remains the same.)  The Snafu Suite now gives Snafu FUSE Volume permission to bind.

When a Snafu FUSE Volume unmounts, it should also notify the Snafu Suite so the preserved mountpoint's resources can be released.  I suppose it's not critical as the preserved mountpoint will continue to function, but with an unnecessesary address translation step.

The Snafu call to request mountpoint permission can in the same call return a valid access path ino to the Snafu FUSE Volume.  The ino is the root ino of the volume.






\section servermodule	Server Module

uses the following databases
- serveraddrdb
