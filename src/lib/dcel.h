#ifndef _INCLUDE_HIENA_DOMAIN_CELL_H_
#define _INCLUDE_HIENA_DOMAIN_CELL_H_

/**
 * @file
 *
 * The Domain Cell stores information about a domain of data.
 *
 * Stores a data map, directory list, file stream, analytical table.
 *
 */


struct hiena_prod {
	void *server;
	void *addr;
	int  *argc;
	void *argv[];
};

struct hiena_map_cel {
	void *off;
	void *len;
	/** grammar rule that created this mapping */
	void *type;

	struct hiena_map_cel *prev;
	struct hiena_map_cel *next;
	/** _anchor_ not _prev_anchor_ because
	  hiena_map_cel and hiena_map_anchor are aligned on the same byte
	  of the underlying fragment.
	  */
	struct hiena_map_anchor *anchor;
	/** maps to the byte _after_ the last byte
	  mapped by the hiena_map_cel. */
	struct hiena_map_anchor *next_anchor;
};

/**
 * A Hiena Map Anchor is created at the head of the fragment, then one byte position after the tail of every created map object.  This byte position becomes the byte position of the next map object.
 *
 * Map objects are created on anchors.
 *
 * Anchors to not need to point at one another.  Their purpose is to link map cels together and to link fragments to map cels.
 *
 * An anchor always links the head of a fragment to the first map object in the fragment.
 * 
 */
struct hiena_map_anchor {
	/** aligned to the same byte of the underlying fragment */
	struct hiena_map_cel *map_cel;
	/** ends on the last byte _before_ this anchor */
	struct hiena_map_cel *prev_map_cel;
	/** previous anchor in the fragment, or none if this is frag head */
	struct hiena_map_anchor *prev;
	/** next anchor in the fragment, or none if this is the frag tail
	 *
	 * the last anchor in a fragment ends one byte _beyond_
	 * the actual fragment EOF.
	 */
	struct hiena_map_anchor *next;
};

enum fragrole { FRAGCONTENT, FRAGCONTAINER, FRAGLIST };

struct hiena_frag {
	int role;
	/** frag content role uses content to hold 'source' grid_addr.

	   frag container role uses content to hold more frags */
	void *content;

	/** frag container role uses content tail.
	  if it's contents are list fragments. */
	void *content_tail;
	struct hiena_frag *prev;
	struct hiena_frag *next;
	
	/** frag content role caches the production instruction */
	void *prod;

	/** frag content role holds map anchors */
	struct hiena_map_anchor *map_head;
	struct hiena_map_anchor *map_tail;

	/** frag content role holds directory entries for it's stream */
	void *dirent;
	void *dirent_tail;
};

/**
 * The Hiena Domain Cell functions as a file stream and as a directory dream, depending on how you access it.  It carries a searchable map of its streams produced by running the Hiena Mapping Engine on its source.
 *
 * The actual dcel structure appears minimal.  All source, mapping and directory entries are carried by the `frag` structure.
 */

struct hiena_dcel {
	void *dcelno;
	/** fragment structure
	  maps out areas of a stream,
	  leads to 'source' grid_addr */
	struct hiena_frag *frag;
};


struct hiena_dcel *new_dcel();
void prod_cleanup( struct hiena_prod *prod );
void frag_cleanup( struct hiena_frag *frag );
void dcel_cleanup( struct hiena_dcel *dcel );
struct hiena_frag *new_frag();
struct hiena_prod *new_prod();
struct hiena_prod *source_prod( char *server, char *addr );
struct hiena_dcel *new_dcel_from_filepath( char *fpath );

#endif /*!_INCLUDE_HIENA_DOMAIN_CELL_H_*/
