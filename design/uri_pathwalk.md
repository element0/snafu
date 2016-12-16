SNAFU PATHWALK {#snafu_pathwalk}
==============

An atomic function, it acts on a `uri` and generates an `access path` (built of `access frames`).


### OBJECTS AND FUNCTIONS ###

	snafu_pathwalk :: CosmosURI -> AccessPath

An access path is built of `access frames`.

	data AccessFrame = { parent :: AccessFrame
			   , fs     :: DCel
			   }

	data DCel = { content :: Fragment
		    }



### STEP BY STEP ###

*ROOT ACCESS FRAME*

First, setup `root access frame`.

	protodcel = new_dcel_from_filepath( COSMOSROOT );	
	dcel   = new_dcel_from_uri( root_of_uri( uri ) );

	parent = NULL;
	cosm   = overloadcosm( loadcosm( protodcel ), loadcosm( dcel ) );

	while(
	dcel   = update_dcel_gridsources( dcel, cosm ) != dcel && !TIMEOUT ) {
	cosm   = overloadcosm( cosm, loadcosm( dcel ) );
	}

	result aframe

(`loadcosm()` scans a `dcel` for `.cosm`, `_cosm` or METADIRNAME. )


*DECENDENT ACCESS FRAMES*

For each `segment` in `uri pathname`:
	
	prev_aframe = aframe;     /* from last iteration, possibly root */
	lookupstr  = segment;
	aframe     = lookup( prev_aframe, lookupstr, lookupmodule );
		     /* sets dcel */

	parent = prev_aframe;
	cosm   = overloadcosm( prev_aframe->cosm, loadcosm( dcel ) );

	while(
	dcel   = update_dcel_gridsources( dcel, cosm ) != dcel && !TIMEOUT ) {
	cosm   = overloadcosm( cosm, loadcosm( dcel ) );
	}

	result aframe



### CACHE DATABASE ###

`new_dcel_*()` and `update_dcel_*()` store primatives in Griddb.
`loadcosm()` and `overloadcosm()` store primatives in Griddb.
`lookup()` loads and stores primatives in Griddb.
`cosmos_pathwalk()` stores each `aframe` in Griddb.



### RETHINKING GRIDDB ###

Griddb should be split between Griduserd and Gridshared.  The choice between domains should be made on a element by element basis depending on its ACL.

The separation between domains should be invisible to the Griddb API.

