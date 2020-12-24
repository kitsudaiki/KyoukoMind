/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include <common/includes.h>

// const predefined values
#define UNINIT_STATE_64 0xFFFFFFFFFFFFFFFF
#define UNINIT_STATE_32 0xFFFFFFFF
#define UNINIT_STATE_24 0xFFFFFF
#define UNINIT_STATE_16 0xFFFF
#define UNINIT_STATE_8 0xFF

// network-predefines
#define SYNAPSES_PER_SYNAPSESECTION 15
#define MAX_NUMBER_OF_SYNAPSE_SECTIONS 100000
#define MINIMUM_NODE_BODER 40
#define MAXIMUM_NODE_BODER 50

// initializing
#define MAX_DISTANCE 100
#define POSSIBLE_NEXT_AXON_STEP 80

// processing
#define NUMBER_OF_PROCESSING_UNITS 1
#define PROCESS_INTERVAL 100000 // usec
