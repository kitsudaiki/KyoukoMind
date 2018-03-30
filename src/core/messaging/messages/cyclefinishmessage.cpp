#include "cyclefinishmessage.h"

namespace KyoukoMind
{

/**
 * @brief CycleFinishMessage::ReplyMessage
 * @param messageId
 * @param site
 */
CycleFinishMessage::CycleFinishMessage(const ClusterID targetClusterId,
                                       const uint32_t messageId,
                                       const uint8_t site) :
    Message(targetClusterId, messageId, site)
{
    m_metaData.type = CYCLEFINISHMESSAGE;
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
