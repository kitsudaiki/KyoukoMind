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
#define MAX_NUMBER_OF_ACTIVE_NODES 50
#define RANDOM_ADD_ACTIVE_NODE 10

#define PROCESS_INTERVAL 3000000
#define MAX_PENDING_VALID_CYCLES 20

// processing-variables
#define POSSIBLE_NEXT_AXON_STEP 0
#define POSSIBLE_NEXT_LEARNING_STEP 0
#define NODE_COOLDOWN 3
#define AXON_PROCESS_BORDER 1.0
#define AXON_FORK_BORDER 5.0

#endif // DEFINES_H
