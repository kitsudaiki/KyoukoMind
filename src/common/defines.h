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

#define NUMBER_OF_PROCESSING_UNITS 1
#define OUTPUT(OUT) std::cout<<(OUT)<<std::endl;

#define EDGES_PER_EDGESECTION 145

#define MAX_LEARNING_LOOPS 10
#define MAX_DISTANCE 100

#define PROCESS_INTERVAL 3000000
#define MAX_PENDING_VALID_CYCLES 20

#define UNINIT_STATE 0xFFFFFFFF
#define OVERPROVISIONING 2

#define EDGE_MEMORIZE_UPDATE 2
#define NEW_FORWARD_EDGE_BORDER 0.1f

// processing-variables
#define POSSIBLE_NEXT_AXON_STEP 50
#define POSSIBLE_NEXT_LEARNING_STEP 0
#define NODE_COOLDOWN 3
#define NUMBER_OF_NODES_PER_CLUSTER 10
#define AXON_PROCESS_BORDER 1.0
#define AXON_FORK_BORDER 5.0

#endif // DEFINES_H
