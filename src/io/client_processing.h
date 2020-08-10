/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef CLIENT_PROCESSING_H
#define CLIENT_PROCESSING_H

#include <kyouko_root.h>
#include <core/processing/objects/container_definitions.h>
#include <core/object_handling/segment.h>

#include <core/processing/external/message_processing.h>
#include <core/processing/cpu/cpu_processing_unit.h>

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

/**
 * @brief clientControlLearning_processing
 * @param content
 */
inline void
clientControlLearning_processing(const ClientControlLearning &content,
                                 KyoukoRoot* rootObject)
{
    // GlobalValues gValues = handler->getGlobalValues();
    // gValues.globalLearningTemp = content.learnTemp;
    // gValues.globalLearningOffset = content.learnOffset;
}

/**
 * @brief clientControlMemorizing_processing
 * @param content
 */
inline void
clientControlMemorizing_processing(const ClientControlMemorizing &content,
                                   KyoukoRoot* rootObject)
{
    // GlobalValues gValues = handler->getGlobalValues();
    // gValues.globalMemorizingTemp = content.memTemp;
    // gValues.globalMemorizingOffset = content.memOffset;
}

/**
 * @brief clientControlGlia_processing
 * @param content
 */
inline void
clientControlGlia_processing(const ClientControlGlia &content,
                             KyoukoRoot* rootObject)
{
    // GlobalValues gValues = handler->getGlobalValues();
    // gValues.globalGlia = content.glia;
}

/**
 * @brief clientControlOutputLearning_processing
 * @param content
 */
inline void
clientControlOutputLearning_processing(const ClientControlOutputLearning &content,
                                       KyoukoRoot* rootObject)
{

    Brick* brick = &getBuffer<Brick>(rootObject->m_segment->bricks)[content.brickId];
    assert(brick != nullptr);

    //brick->learningOverride = content.outputLearning;
}

/**
 * @brief clientLearnInput_processing
 * @param content
 */
inline void
clientLearnInput_processing(const ClientLearnInputData &content,
                            KyoukoRoot* rootObject)
{
    std::map<uint32_t, Brick*>::const_iterator it;
    it = rootObject->m_inputBricks->find(content.brickId);
    if(it != rootObject->m_inputBricks->end())
    {
        Brick* brick = it->second;
        const uint16_t ok = brick->neighbors[22] != UNINIT_STATE_32;
        // TODO
    }
}

/**
 * @brief clientLearnFinishCycleData_processing
 * @param content
 */
inline void
clientLearnFinishCycleData_processing(const ClientLearnFinishCycleData &content,
                                      KyoukoRoot* rootObject)
{
    std::map<uint32_t, Brick*>::const_iterator it;
    it = rootObject->m_inputBricks->find(content.brickId);
    if(it != rootObject->m_inputBricks->end())
    {
        Brick* brick = it->second;
    }
}

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
    KyoukoRoot* rootObject = static_cast<KyoukoRoot*>(target);
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
                clientControlLearning_processing(content, rootObject);
                dataPos += sizeof(ClientControlLearning);
                break;
            }

            case CLIENT_CONTROL_MEMORIZING:
            {
                //LOG_DEBUG("CLIENT_CONTROL_MEMORIZING");
                const ClientControlMemorizing content =
                        *((ClientControlMemorizing*)&dataObj[dataPos]);
                clientControlMemorizing_processing(content, rootObject);
                dataPos += sizeof(ClientControlMemorizing);
                break;
            }

            case CLIENT_CONTROL_GLIA:
            {
                //LOG_DEBUG("CLIENT_CONTROL_GLIA");
                const ClientControlGlia content = *((ClientControlGlia*)&dataObj[dataPos]);
                clientControlGlia_processing(content, rootObject);
                dataPos += sizeof(ClientControlGlia);
                break;
            }

            case CLIENT_CONTROL_OUTPUT_LEARNING:
            {
                //LOG_DEBUG("CLIENT_CONTROL_OUTPUT_LEARNING");
                const ClientControlOutputLearning content
                        = *((ClientControlOutputLearning*)&dataObj[dataPos]);
                clientControlOutputLearning_processing(content, rootObject);
                dataPos += sizeof(ClientControlOutputLearning);
                break;
            }

            case CLIENT_LEARN_INPUT:
            {
                //LOG_DEBUG("CLIENT_LEARN_INPUT");
                const ClientLearnInputData content = *((ClientLearnInputData*)&dataObj[dataPos]);
                clientLearnInput_processing(content, rootObject);
                dataPos += sizeof(ClientLearnInputData);
                break;
            }

            case CLIENT_LEARN_FINISH_CYCLE:
            {
                //LOG_DEBUG("CLIENT_LEARN_FINISH_CYCLE");
                const ClientLearnFinishCycleData content =
                        *((ClientLearnFinishCycleData*)&dataObj[dataPos]);
                clientLearnFinishCycleData_processing(content, rootObject);
                dataPos += sizeof(ClientLearnFinishCycleData);
                break;
            }

            default:
                assert(false);
                break;
        }
    }
}

#endif // CLIENT_PROCESSING_H
