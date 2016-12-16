PRODUCTION INSTRUCTION {#production_instruction}
======================

A production instruction creates a Dcel.  It is the "Generative Grammar" of the Domain.

There are a set of producers
- sourcerer
- binder
- deviner
- alchemist

aka
- source	generates a dcel based on a single source address
- bind		generates a dcel by combining other dcels
- find		generates a dcel from a selection of fragments
- grind		generates a dcel from a generative expression


A production instruction is "grid capable" it can be sent as a grid instruction and exectuted in parallel.

	source SERVER ADDRESS

	bind [METHOD] DCEL DCEL ...

	find QUERY

	grind EXPRESSION


Structure:

The structure of a production instruction object:

	struct production_instruction {
		void *server;
		void *addr;
		int  *argc;
		void *argv[];
	};

	data Prod = Prod {
		server   :: Ptr,
		addr	 :: Prt,
		producer :: String,
		args 	 :: [a]
	}

in other words a production instruction contains a string of strings just like a command line argument.
	
	Prod :: [ String ]


	source file "/"


\section prod_inst_api		Production Instruction API

production instruction api.

	new()
	set_server

