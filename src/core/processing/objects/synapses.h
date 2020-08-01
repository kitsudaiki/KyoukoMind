/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef SYNAPSES_H
#define SYNAPSES_H

#include <common.h>

namespace KyoukoMind
{

//==================================================================================================

struct Synapse
{
    float weight = 0.0;
    float memorize = INITIAL_MEMORIZING;
    uint16_t targetNodeId = UNINIT_STATE_16;
    uint8_t inProcess = 0;
    uint8_t somaDistance = 1;
    // total size: 12 Byte
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

} // namespace KyoukoMind

#endif // SYNAPSES_H
