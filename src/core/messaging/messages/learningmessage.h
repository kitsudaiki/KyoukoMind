#ifndef LEARNINGMESSAGE_H
#define LEARNINGMESSAGE_H

#define MAX_NUMBER_OF_NEW_EDGES 100

#include "message.h"

namespace KyoukoMind
{

class LearningMessage : public Message
{
public:
    LearningMessage(const uint32_t clusterId,
                    const uint32_t messageIdCounter,
                    const uint8_t site);
    LearningMessage();

    bool convertFromByteArray(uint8_t* data);
    uint8_t* convertToByteArray();

    bool addNewEdge(const KyoChanNewEdge& newEdge);
    uint8_t getNumberOfEdges() const;
    KyoChanNewEdge* getNewEdges() const;

private:
    uint8_t m_maxNumberOfNewEdges = MAX_NUMBER_OF_NEW_EDGES;
    uint8_t m_numberOfNewEdges = 0;
    KyoChanNewEdge m_newEdges[MAX_NUMBER_OF_NEW_EDGES];
};

}

#endif // LEARNINGMESSAGE_H
