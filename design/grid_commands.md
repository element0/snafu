Grid Commands	{#grid_commands}
-------------

grid Commands are ubiquitous in form between both User and Host.

A grid command is a subset of a unix command line command.

Only grid commands are cataloged within the grid library.

Grid commands must have meta data to know how they can or should be distributed.

**Command Line Commands**

A bare unix command line command could be considered to be a grid command without distributibility.  In other words a command line command without a "gridability profile" can only be expected to run on a single gridhost.

A bare unix command line is acceptable as a grid command.  It will run on the local host -- subject to the normal stipulations of the command's availability in the environment.


**Gridability**

A command line command with a "gridability profile" has been analized for "factorability".  The Gridability Profile directs the Grid Instruction Engine how to factor the command line into a table of command lines, each line prepared for a different host.


**Built-in Vs. Library**

Grid command executible can be built-in to the Grid Instruction Interpreter.  Or it can exist in the host environment.


**Factorability**

- terminal
- sequential aggregate

<WORKING...>
