/** @file
 * Copyright (C) 2016  Raygan Henley
 * All Rights Reserved.
 *
 * Interpreter for Gridmessages received by Griduser program.
 */

#include "cosmos_grid.h"
#include "buflink.h"

/**
 * Executes a `gridmsg` in the context of the `griduser` program.
 * Meaning: I expect `griduser` to run in user space.
 * And the vocabulary of the grid messages will depend on the 
 * vocabulary of the griduser program.
 */
void *gridmsg_exec( struct cosmos_grid *g, struct buflink *buf );

