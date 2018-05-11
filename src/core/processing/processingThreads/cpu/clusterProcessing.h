/**
 *  @file    clusterProcessing.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef CLUSTERPROCESSING_H
#define CLUSTERPROCESSING_H

#include <common.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/messageContainer.h>

namespace KyoukoMind
{
class OutgoingMessageBuffer;
class EdgeCluster;
class NodeCluster;
class NextChooser;

class ClusterProcessing
{
public:
    ClusterProcessing(NextChooser *nextChooser);

    void initLearing(KyoChanForwardEdgeSection *currentSection,
                     const float weightDiff);

    void createNewEdgeForward(EdgeCluster *cluster,
                              const uint32_t sourceEdgeClusterId,
                              const float weight,
                              OutgoingMessageBuffer *outgoBuffer);

    void processEdgeForwardSection(KyoChanForwardEdgeSection *currentSection,
                                   const float weight,
                                   OutgoingMessageBuffer *outgoBuffer);
    void processEdgeSection(KyoChanEdgeSection *currentSection,
                            const float weight,
                            KyoChanNode *nodes);

private:
    NextChooser* m_nextChooser = nullptr;

};

}

#endif // CLUSTERPROCESSING_H
