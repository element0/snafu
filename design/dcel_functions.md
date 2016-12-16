DCEL FUNCTIONS {#dcel_functions}
==============

[TOC]


`new_dcel_from_filepath( pathstr )`

translates `pathstr` into a `file://` `uristr` and calls `new_dcel_from_uri()`


`new_dcel_from_uri( uristr )`

	(
	( dcel = new_dcel() )
	->frag = new_frag() )
	->prod = source_prod( uristr );

	/* source_prod() produces
	   prod->serv = servermod_by_name( serverlib, uri_scheme( uristr ) );
	 */

	result dcel

`update_dcel_gridsources( dcel, cosm )`

get a list of gridsources from `cosm`.  `gridsources` is a list of `uri's`.

	list_add(dcels, dcel)
	for each in gridsources
		list_add( dcels, new_dcel_from_uri( each ) );
	
	(
	( dcel = new_dcel() )
	->frag = new_frag() )
	->prod = bind_prod( dcels );
		
	result dcel

	(if gridsources is empty, return original dcel)

\section  dcel_in_c	Dcel Implementation in C

	struct prod_instr {
		void *serv;
		void *addr;
	};
	new_prod();

	struct cosmos_frag {
		int role;
		struct cosmos_frag *content;
		struct cosmos_frag *content_tail;
		struct cosmos_frag *prev;
		struct cosmos_frag *next;
		struct cosmos_map_anchor *map_head;
		struct cosmos_map_anchor *map_tail;
		void *dirent;
		void *dirent_tail;
		struct prod_instr *prod;
	};
	new_frag();

	struct cosmos_dcel {
		struct cosmos_frag *frag;
	};
	new_dcel();

\section  dcel_in_cpp	Dcel Implementation in C++

\section  dcel_in_go	Dcel Implementaion in Go



\section  dcel_in_haskell	Dcel Implementation in Haskell

	data Dcel = Dcel { dno  :: Integer
			 , frag :: Frag }

	data Frag = Container [ Frag ]
	          | Content { prod :: Prod
			    , maph :: MapAnchor
			    , mapt :: MapAnchor
			    , dir  :: [ Dirent ] }

	data Prod = Prod { serv :: Server
			 , addr :: Address }

	data Dirent = ( String, Integer )

