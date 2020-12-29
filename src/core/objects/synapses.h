/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef SYNAPSES_H
#define SYNAPSES_H

#include <common.h>


enum SynapseType
{
    UNDEFINED_SYNAPSE_TYPE = 0,
    POSITIVE_TYPE = 1,
    NEGATIVE_TYPE = 2,
};

//==================================================================================================

struct Synapse
{
    float staticWeight = 0.0;
    float dynamicWeight = 0.0;
    float memorize = 0.90f;
    uint16_t targetNodeId = UNINIT_STATE_16;
    uint8_t inProcess = 0;
    uint8_t type = SynapseType::UNDEFINED_SYNAPSE_TYPE;
    // total size: 16 Byte
};

//==================================================================================================

struct SynapseSection
{
    uint8_t status = ACTIVE_SECTION;
    uint8_t positionInEdge = UNINIT_STATE_8;

    uint16_t randomPos = UNINIT_STATE_16;

    uint32_t sourceEdgeId = UNINIT_STATE_32;
    uint32_t sourceBrickId = UNINIT_STATE_32;

    // has to be at least a very small value to avoid division by zero
    float totalWeight = 0.0000001f;

    Synapse synapses[SYNAPSES_PER_SYNAPSESECTION];


    SynapseSection()
    {
        for(uint32_t i = 0; i < SYNAPSES_PER_SYNAPSESECTION; i++)
        {
            Synapse newSynapse;
            synapses[i] = newSynapse;
        }
    }
    // total size: 256 Byte
};

//==================================================================================================

#endif // SYNAPSES_H
