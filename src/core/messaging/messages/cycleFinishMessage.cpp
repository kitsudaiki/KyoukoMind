/**
 *  @file    cycleFinishMessage.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "cycleFinishMessage.h"

namespace KyoukoMind
{

/**
 * @brief CycleFinishMessage::ReplyMessage
 * @param messageId
 * @param site
 */
CycleFinishMessage::CycleFinishMessage(const ClusterID targetClusterId,
                                       const uint32_t messageId,
                                       const uint8_t targetSite) :
    Message(targetClusterId, messageId, targetSite)
{
    m_metaData.type = CYCLE_FINISH_MESSAGE;
    m_metaData.requiredReply = 1;
}

/**
 * @brief CycleFinishMessage::ReplyMessage
 */
CycleFinishMessage::CycleFinishMessage() : Message()
{}

/**
 * @brief CycleFinishMessage::convertFromByteArray
 * @param data
 * @return
 */
bool CycleFinishMessage::convertFromByteArray(uint8_t *data)
{
    if(data == nullptr) {
        return false;
    }
    convertCommonFromByteArray(data);
    return true;
}

/**
 * @brief CycleFinishMessage::convertToByteArray
 * @return
 */
uint8_t* CycleFinishMessage::convertToByteArray()
{
    return convertCommonToByteArray(0);
}

}
