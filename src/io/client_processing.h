#ifndef CLIENT_PROCESSING_H
#define CLIENT_PROCESSING_H

#include <root_object.h>
#include <core/objects/container_definitions.h>
#include <core/processing/processing_methods/brick_initializing_methods.h>
#include <core/processing/processing_methods/brick_processing_methods.h>
#include <core/processing/processing_methods/neighbor_methods.h>
#include <core/processing/processing_methods/network_segment_methods.h>

#include <libKitsunemimiProjectNetwork/session.h>
#include <libKitsunemimiProjectNetwork/session_controller.h>
#include <libKitsunemimiPersistence/logger/logger.h>

#include <libKitsunemimiKyoukoCommon/communication_structs/client_contianer.h>

using Kitsunemimi::Kyouko::ClientControlLearning;
using Kitsunemimi::Kyouko::ClientControlMemorizing;
using Kitsunemimi::Kyouko::ClientControlGlia;
using Kitsunemimi::Kyouko::ClientControlOutputLearning;
using Kitsunemimi::Kyouko::ClientLearnInputData;
using Kitsunemimi::Kyouko::ClientLearnFinishCycleData;

namespace KyoukoMind
{

/**
 * @brief streamDataCallback
 * @param target
 * @param data
 * @param dataSize
 */
void
clientCallback(void* target,
               Kitsunemimi::Project::Session*,
               const void* data,
               const uint64_t dataSize)
{
    RootObject* rootObject = static_cast<RootObject*>(target);
    const uint8_t* dataObj = static_cast<const uint8_t*>(data);

    //LOG_DEBUG("process incoming client message with size: " + std::to_string(dataSize));

    uint64_t dataPos = 0;

    while(dataPos < dataSize)
    {
        const uint8_t type = dataObj[dataPos];

        switch(type)
        {
            case CLIENT_CONTROL_LEARNING:
            {
                //LOG_DEBUG("CLIENT_CONTROL_LEARNING");
                const ClientControlLearning content = *((ClientControlLearning*)&dataObj[dataPos]);

                GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;

                GlobalValues gValues = handler->getGlobalValues();
                gValues.globalLearningTemp = content.learnTemp;
                gValues.globalLearningOffset = content.learnOffset;

                handler->setGlobalValues(gValues);

                dataPos += sizeof(ClientControlLearning);
                break;
            }

            case CLIENT_CONTROL_MEMORIZING:
            {
                //LOG_DEBUG("CLIENT_CONTROL_MEMORIZING");
                ClientControlMemorizing content = *((ClientControlMemorizing*)&dataObj[dataPos]);

                GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;

                GlobalValues gValues = handler->getGlobalValues();
                gValues.globalMemorizingTemp = content.memTemp;
                gValues.globalMemorizingOffset = content.memOffset;

                handler->setGlobalValues(gValues);

                dataPos += sizeof(ClientControlMemorizing);
                break;
            }

            case CLIENT_CONTROL_GLIA:
            {
                //LOG_DEBUG("CLIENT_CONTROL_GLIA");
                const ClientControlGlia content = *((ClientControlGlia*)&dataObj[dataPos]);

                GlobalValuesHandler* handler = rootObject->m_globalValuesHandler;

                GlobalValues gValues = handler->getGlobalValues();
                gValues.globalGlia = content.glia;

                handler->setGlobalValues(gValues);

                dataPos += sizeof(ClientControlGlia);
                break;
            }

            case CLIENT_CONTROL_OUTPUT_LEARNING:
            {
                //LOG_DEBUG("CLIENT_CONTROL_OUTPUT_LEARNING");
                const ClientControlOutputLearning content
                        = *((ClientControlOutputLearning*)&dataObj[dataPos]);

                Brick* bricks = getBrickBlock(*RootObject::m_segment);
                Brick* brick = &bricks[content.brickId];
                assert(brick != nullptr);

                brick->learningOverride = content.outputLearning;

                dataPos += sizeof(ClientControlOutputLearning);
                break;
            }

            case CLIENT_LEARN_INPUT:
            {
                //LOG_DEBUG("CLIENT_LEARN_INPUT");
                const ClientLearnInputData content = *((ClientLearnInputData*)&dataObj[dataPos]);

                std::map<uint32_t, Brick*>::const_iterator it;
                it = rootObject->m_inputBricks->find(content.brickId);
                if(it != rootObject->m_inputBricks->end())
                {
                    Brick* brick = it->second;
                    Neighbor* neighbor = &brick->neighbors[22];
                    const uint16_t ok = neighbor->targetBrick->nodePos >= 0;

                    for(uint16_t i = 0; i < ok * NUMBER_OF_NODES_PER_BRICK; i++)
                    {
                        assert(neighbor->outgoingBuffer != nullptr);

                        DirectEdgeContainer newEdge;
                        newEdge.weight = content.value;
                        newEdge.targetNodeId = i;
                        assert(neighbor->outgoingBuffer != nullptr);
                        Kitsunemimi::addObject_StackBuffer(*neighbor->outgoingBuffer, &newEdge);
                    }
                }

                dataPos += sizeof(ClientLearnInputData);
                break;
            }

            case CLIENT_LEARN_FINISH_CYCLE:
            {
                //LOG_DEBUG("CLIENT_LEARN_FINISH_CYCLE");
                const ClientLearnFinishCycleData content =
                        *((ClientLearnFinishCycleData*)&dataObj[dataPos]);

                std::map<uint32_t, Brick*>::const_iterator it;
                it = rootObject->m_inputBricks->find(content.brickId);
                if(it != rootObject->m_inputBricks->end())
                {
                    Brick* brick = it->second;
                    assert(brick->neighbors[22].outgoingBuffer != nullptr);
                    finishSide(brick, 22);
                    while(isReady(brick) == false) {
                        usleep(1000);
                    }
                    initCycle(brick);
                }

                dataPos += sizeof(ClientLearnFinishCycleData);
                break;
            }

            default:
                assert(false);
                break;
        }
    }
}

}

#endif // CLIENT_PROCESSING_H
