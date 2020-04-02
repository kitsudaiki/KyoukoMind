/**
 *  @file    defines.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <common/includes.h>

#define OUTPUT(OUT) std::cout<<(OUT)<<std::endl;
//#define RUN_UNIT_TEST

// const predefined values
#define UNINIT_STATE_64 0xFFFFFFFFFFFFFFFF
#define UNINIT_STATE_32 0xFFFFFFFF
#define UNINIT_STATE_16 0xFFFF

// network-predefines
#define EDGES_PER_SYNAPSESECTION 64
#define NUMBER_OF_NODES_PER_BRICK 10
#define MINIMUM_NODE_BODER 20
#define MAXIMUM_NODE_BODER 50

// initializing
#define MAX_DISTANCE 100
#define POSSIBLE_NEXT_AXON_STEP 80

// learning
#define NEW_FORWARD_EDGE_BORDER 0.1f
#define NEW_SYNAPSE_BORDER 1.0f
#define DELETE_SYNAPSE_BORDER 1.0f
#define MINIMUM_NEW_EDGE_BODER 1.0f
#define MAX_SOMA_DISTANCE 5

// memorizing
#define INITIAL_MEMORIZING 0.5f

// processing
#define NUMBER_OF_PROCESSING_UNITS 1
#define PROCESS_INTERVAL 100000 // usec
#define NODE_COOLDOWN 3.0f
#define ACTION_POTENTIAL 100.0f
#define REFRACTION_TIME 2
#define PROCESSING_MULTIPLICATOR 1.0f

// messages
#define MESSAGES_PER_BLOCK 1024
#define MESSAGE_SIZE 512
#define INCOMING_BUFFER_SIZE 5
