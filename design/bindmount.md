Bind Mount {#bindmount}
==========

[TOC]

A snafu FUSE volume is designed to mount over an `non-empty mountpoint`.

This requires:

- preserve the existing filesystem at mountpoint



\section preserve_mountpoint	Preserve Filesystem Under Mountpoint


The configuration provides a `GRIDUSERHOST_VAR_DIR`.  In that directory we save `preservation mounts`.

The First Draft Implementation uses `bindfs`: a capable -- but [it's got quirks](@ref bindfs_quirks) -- FUSE file system that basically clones a subdirectory.

	PRESERVED_PATH=${GRIDUSERHOST_VAR_DIR}/preserved${NUM}.d
	mkdir ${PRESERVED_PATH}
	bindfs ${mountpoint} ${PRESERVED_PATH}
	
And when the snafu volume exits:

	fusermount -u ${PRESERVED_PATH}



\section  access_preserved	Access Preserved Filesystem


The `griduserhost` stores an `access_frame` in the `griddb` for each path segment in the localhost file tree as it is accessed.

An access frame has one direct dcel and zero or more indirect dcels.  Each dcel has a fragment map, each fragment points to a data source address.

A source address is at minimum a server:address pair.

We create a new dcel out of serveraddresspair file:preservationaddr instead of file:mountpointaddr.

And we bind the dcel with any existing dcels in the access frame. 



\section  test_movesource_postmount  Tested: Move Source Post-Mount


The highest and best is if we could refer to the actual inode of the source file system.  That way if a mountpath moved, we would still be linked.

	touch ~redboy/project/zone1/barndoor.txt

	snafu ~redboy/project/zone1

	mv ~redboy/project/zone1 ~redboy/archive/zone1

	ls ~redboy/archive/zone1

We hope it lists `barndoor.txt`

As tested using `bindfs`, I can't move the mountpoint after mounting.  It says `resource busy`.

But I _can_ move a source for `bindfs` after mounting to a different mountpoint without breaking the links.  good.




\section  bindfs_quirks		Bindfs (Third Party) Quirks

`bindfs` quirks are a function of how FUSE works.  if multiple users access the bindfs mountpoint, all file creation is attributed to the mount owner, who may not be the current user.  This only makes it acceptable for read only mounts in a multiple user environment.

The use of Bindfs is temporary.  We should roll our own version to clone a subdirectory tree, but handle permissions through the `griduserhost`.

\section  fuse_quirks		FUSE (Third Party) Quirks

Fusebug #15.  According to its documentation is attributable to the kernel VFS.  Sometimes permissions are cached by the kernel and don't update in the file system.  Thereby permitting inappropriate access.

This affects multi-user FUSE mounts.

