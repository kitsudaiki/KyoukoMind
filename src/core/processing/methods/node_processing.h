#ifndef NODE_PROCESSING_H
#define NODE_PROCESSING_H

#include <common.h>
#include <root_object.h>

#include <core/objects/brick.h>
#include <core/objects/container_definitions.h>

#include <core/processing/methods/edge_container_processing.h>
#include <core/methods/brick_item_methods.h>
#include <core/methods/neighbor_methods.h>
#include <core/methods/network_segment_methods.h>

namespace KyoukoMind
{

//==================================================================================================
/**
 * processing of the nodes of a specific node-brick
 *
 * @return number of active nodes in this brick
 */
inline uint16_t
processNodes(NetworkSegment &segment,
             Brick &brick)
{
    if(brick.isOutputBrick != 0) {
        return 0;
    }

    uint16_t numberOfActiveNodes = 0;

    // process nodes
    Node* start = &getNodeBlock(segment)[brick.nodePos];
    Node* end = start + NUMBER_OF_NODES_PER_BRICK;

    // iterate over all nodes in the brick
    for(Node* node = start;
        node < end;
        node++)
    {
        // set to 255.0f, if value is too high
        if(node->currentState > 255.0f) {
            node->currentState = 255.0f;
        }

        // init
        const Node tempNode = *node;

        // check if active
        if(tempNode.border <= tempNode.currentState
                && tempNode.refractionTime == 0)
        {
            node->potential = ACTION_POTENTIAL;
            numberOfActiveNodes++;
            node->active = 1;
            node->refractionTime = REFRACTION_TIME;
        }
        else if(tempNode.refractionTime == 0) {
            node->active = 0;
        }

        // forward current potential
        if(node->active == 1)
        {
            const float weight = node->potential * PROCESSING_MULTIPLICATOR;
            const uint64_t path = tempNode.targetBrickPath/32;

            AxonEdgeContainer newEdge;
            newEdge.targetAxonId = tempNode.targetAxonId;
            newEdge.targetBrickPath = path;
            newEdge.weight = weight;

            processAxon(segment, brick, newEdge);
        }
        // post-steps
        node->refractionTime = node->refractionTime >> 1;

        // set to 0.0f, if value is negative
        if(node->currentState < 0.0f) {
            node->currentState = 0.0f;
        }

        // check if node-state is too high compared to the border
        node->tooHigh = node->currentState > 1.2f * node->border;

        // make cooldown in the node
        node->potential /= NODE_COOLDOWN;
        node->currentState /= NODE_COOLDOWN;
    }

    return numberOfActiveNodes;
}

//==================================================================================================

/**
 * @brief NodeBrick::memorizeEdges
 */
inline void
postLearning(NetworkSegment &segment,
             Brick &brick)
{
    SynapseSection* sectionStart = getSynapseSectionBlock(segment);
    SynapseSection* sectionEnd = sectionStart + segment.synapses.numberOfItems;

    // iterate over all edge-sections
    for(SynapseSection* section = sectionStart;
        section < sectionEnd;
        section++)
    {
        if(section->status != ACTIVE_SECTION) {
            continue;
        }

        // update values based on the memorizing-value
        Synapse* end = section->synapses + section->numberOfSynapses;
        for(Synapse* synapse = section->synapses;
            synapse < end;
            synapse++)
        {

            // normal learn
            /*if(edge->inProcess == 1
                    && m_globalValue.globalLearningTemp != 0.0f
                    && edge->targetNodeId != NUMBER_OF_NODES_PER_BRICK - 1)
            {
                const float diff = 1.0f - edge->memorize;
                edge->weight += (diff * m_globalValue.globalLearningTemp);
            }*/

            // mem
            if(brick.globalValues.globalMemorizingTemp != 0.0f)
            {
                synapse->memorize = brick.globalValues.globalMemorizingTemp;
                if(synapse->memorize > 1.0f) {
                    synapse->memorize = 1.0f;
                }
            }
        }
    }
}

//==================================================================================================

/**
 * @brief memorizeEdges
 * @param brick
 */
inline void
memorizeSynapses(NetworkSegment &segment,
                 Brick &brick)
{
    SynapseSection* sectionStart = getSynapseSectionBlock(segment);
    SynapseSection* sectionEnd = sectionStart + segment.synapses.numberOfItems;

    // iterate over all synapse-sections
    uint32_t sectionPos = 0;
    for(SynapseSection* synapseSection = sectionStart;
        synapseSection < sectionEnd;
        synapseSection++)
    {
        if(synapseSection->sourceBrickId != brick.brickId){
            continue;
        }

        // skip if section is deleted
        if(synapseSection->status != ACTIVE_SECTION)
        {
            sectionPos++;
            continue;
        }

        // update values based on the memorizing-value
        Synapse* end = synapseSection->synapses + synapseSection->numberOfSynapses;
        for(Synapse* synapse = synapseSection->synapses;
            synapse < end;
            synapse++)
        {
            if(synapse->inProcess == 1) {
                continue;
            }

            const float newWeight = synapse->weight * (1.0f - synapse->memorize);
            synapse->weight -= newWeight;
            synapseSection->totalWeight -= newWeight;
        }

        // delete dynamic item if value is too low
        EdgeSection* edgeSection = &getEdgeBlock(brick)[synapseSection->sourceEdgeId];
        assert(edgeSection->status == ACTIVE_SECTION);

        if(synapseSection->totalWeight < DELETE_SYNAPSE_BORDER)
        {
            deleteDynamicItem(segment.synapses, sectionPos);
            processUpdateDeleteEdge(brick, *edgeSection, synapseSection->sourceEdgeId, 22);
        }
        else
        {
            const float updateValue = synapseSection->totalWeight;
            processUpdateSetEdge(brick, *edgeSection, updateValue, 22);
        }

        sectionPos++;
    }
}

//==================================================================================================

}

#endif // NODE_PROCESSING_H
