/**
 *  @file    learningMessage.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef LEARNINGMESSAGE_H
#define LEARNINGMESSAGE_H

#define MAX_NUMBER_OF_NEW_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class LearningMessage : public Message
{
public:
    LearningMessage(const ClusterID targetClusterId,
                    const uint32_t sourceClusterId,
                    const uint32_t messageIdCounter,
                    const uint8_t targetSite);
    LearningMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();

    bool addNewEdge(const KyoChanNewEdge& newEdge);
    uint8_t getNumberOfNewEdges() const;
    KyoChanNewEdge* getNewEdges() const;

private:
    uint8_t m_maxNumberOfNewEdges = MAX_NUMBER_OF_NEW_EDGES;
    uint8_t m_numberOfNewEdges = 0;
    KyoChanNewEdge m_newEdges[MAX_NUMBER_OF_NEW_EDGES];
};

}

#endif // LEARNINGMESSAGE_H
