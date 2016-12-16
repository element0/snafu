Access Frames {#accessframe}
=============

Anatomy:

	aframe/
		cosm/
			bin/
				%{cmd}
				%{cmd}
				%{cmd}
			usr/bin/
				%{cmd}
			usr/local/bin/
				%{cmd}
			flows.txt
		env/
			PATH="/usr/local/bin:/usr/bin:/bin"
			%{var}
			%{var}
			%{var}

		fs/
			.cosm/	--> aframe/vroot
			%{node}
			%{node}
			%{node}

		lookup_cache/
			%{lookup_str_id} --> %{aframe_id}
			%{lookup_str_id} --> %{aframe_id}
			%{lookup_str_id} --> %{aframe_id}

		parent_aframe


**An Explanation of an Aframe**

At its root level, the aframe has some interesting directories:

	aframe/
		cosm/
		env/
		fs/
		lookup_cache/
		parent_aframe

These are private to the aframe structure and are never revealed directly to the user.

They each require a little different treatment.

**cosm**
	aframe/
		cosm/

The `aframe/cosm` structure is managed by Cosmos.  It can be read and written, using the `.cosm` or `_cosm` directory name inside any regular directory.

It's contents are persistent accross lookup calls.  But the contents are cascaded from one directory to another.  Changing a cosm in a shallower directory will ripple down to the deeper directories.

**env**
	aframe/
		env/

The `aframe/env` is at worst, confusing.  And at best, awesome.  In the same regard as the cosm cascades downward, the env cascades as well.  It is an interface available to the `lookup module`.  But changes to it only persist during the duration of the lookup pathwalk.  It's content is initialized by the settings inside `.cosm/etc/env`

**fs**
	aframe/
		fs/
Basically, the dcel's directory structure is available through the `fs`.  It's contents persist between lookups.  It represents the file system at the aframe's branch.
		
