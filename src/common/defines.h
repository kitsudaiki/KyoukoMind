/**
 * @file        defines.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
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
#define MINIMUM_NODE_BODER 20
#define MAXIMUM_NODE_BODER 50

// initializing
#define MAX_DISTANCE 100
#define POSSIBLE_NEXT_AXON_STEP 80

// processing
#define NUMBER_OF_PROCESSING_UNITS 1
#define PROCESS_INTERVAL 100000 // usec
