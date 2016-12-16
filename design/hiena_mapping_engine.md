Hiena Mapping Engine
====================

\section mapanchors	Map Anchors

An anchor always links the head of a fragment to the first map object in the fragment.

A tail_anchor links the end of the fragment to the last map object in the fragment.

The mapping engine does not create Map Anchors that have no entry point.


\section seeking	Seeking

Map Anchors serve the purpose of the Hiena Mapping Engine.  As the engine "reads" the dcel it seeks position in both the existing map and the underlying fragment source.

The seek action begins inside the frag, then enters the head map anchor, then enters the first map cell, may seek perpendicular into another aligned mapcel, or may seek into the map cel.  Then enters the `next_anchor`.

The seek continues into the next mapcel or if unsatisfied, into the stream, creating another map cell, creating a `next_anchor`, if the stream shows EOF the last anchor will remain incomplete.

\section fragmentation	Fragmentation

A `hiena_dcel` begins quite simply as having only one fragment container and one fragment content.

	dcel:
		fragment:
			role: container
			content:
				- role: content
				  source: file:///home/raygan/testfile.yaml
		prodinstr: "sourcerer file:///home/raygan/testfile.yaml"

Let's describe the `testfile.yaml`

~~~~~yaml
shape: box
color: blue
~~~~~

The Hiena Mapping engine begins inside the fragment.  It drills into the first `content` fragment it finds.  It looks for a mapanchor.  None.  Reads from source.  maps a `key` maps a `value`.  reduces to `kvpair` maps it. recognizes newline, maps `ws`. maps a `key`, a `value`, reduces... maps... gets to EOF.

	fragment: { first_map_anchor: 1, last_map_anchor: 6 }
		map_anchor: 1
			map_cel: { off: 0, len: 10, type: kvpair,
					 next_anchor: 3 }
			map_cel: { off: 0, len: 7, type: key,
					 next_anchor: 2 }
		map_anchor: 2
	        	map_cel: { off: 7, len: 3, type: value,
					next_anchor: 3
		map_anchor: 3
			map_cel: { off:10, len: 1, type: ws,
					next_anchor: 4 }
		map_anchor: 4
			map_cel: { off:11, len: 11, type: kvpair,
					next_anchor: 6 }
			map_cel: { off:11, len: 7, type: key,
					 next_anchor: 5 }
		map_anchor: 5
			map_cel: { off:11, len: 4, type: value,
					next_anchor: 6 }
	        map_anchor: 6
		   

