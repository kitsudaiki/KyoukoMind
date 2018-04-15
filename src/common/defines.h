/**
 *  @file    defines.h
 *  @author  Tobias Anker
 *  @version 0.0.0
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */


#ifndef DEFINES_H
#define DEFINES_H

#include <common/includes.h>

#define EDGES_PER_EDGESECTION 51
#define MAX_EDGESECTIONS_PER_AXON 100
#define MAX_PENDING_EDGES 10

#define PROCESS_INTERVAL 10000000

// processing-variables
#define POSSIBLE_NEXT_AXON_STEP 50
#define POSSIBLE_NEXT_LEARNING_STEP 50
#define NODE_COOLDOWN 3
#define AXON_PROCESS_BORDER 1.0
#define MAX_PENDING_VALID_CYCLES 20

#endif // DEFINES_H
