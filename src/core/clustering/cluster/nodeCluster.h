/**
 *  @file    nodeCluster.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef NODECLUSTER_H
#define NODECLUSTER_H

#include "cluster.h"

namespace KyoukoMind
{

class NodeCluster : public Cluster
{

public:
    NodeCluster(const ClusterID clusterId,
                const uint32_t x,
                const uint32_t y,
                const std::string directoryPath,
                const uint32_t numberOfNodes = 0);

    // nodes
    uint32_t getNumberOfNodeBlocks() const;
    uint32_t getNumberOfNodes() const;
    KyoChanNode* getNodeBlock();
    bool initNodeBlocks(uint32_t numberOfNodes);

    // edge-sections
    uint32_t getNumberOfEdgeSectionBlocks() const;
    uint32_t getNumberOfEdgeSections() const;
    KyoChanEdgeSection* getEdgeSectionBlock();
    bool initEdgeSectionBlocks(const uint32_t numberOfEdgeSections);
    uint32_t addEmptyEdgeSection(const uint8_t sourceSide,
                                 const uint32_t sourceId);
    bool addEdge(const uint32_t edgeSectionId, const KyoChanEdge &newEdge);

    float getSummedValue();
    void setAsOutput();

    // processing
    bool processNodeCluster();
    uint16_t processNodes();
    void memorizeEdges();
    void postLearning();

protected:
    uint32_t m_memorizeCounter = 0;

    // processing-methods for node-cluster
    void learningEdgeSection(KyoChanEdgeSection *currentSection,
                             float weight,
                             const uint32_t edgeSectionId);
    void processUpdateSetEdge(KyoChanEdgeSection* currentSection,
                              float updateValue);
    void processUpdateSubEdge(KyoChanEdgeSection* currentSection,
                              float updateValue);
    void processUpdateEdge(const uint32_t edgeSectionId,
                           const float updateValue,
                           const uint8_t updateType);
    float checkEdge(KyoChanEdgeSection *currentSection,
                    const float weight, const uint32_t edgeSectionId);
    void processLerningEdge(const uint32_t sourceEdgeSectionId,
                            const float weight,
                            const uint8_t initSide);
    void processPendingEdge(const uint32_t sourceId,
                            const uint8_t sourceSide,
                            const float weight);
    void createNewEdge(KyoChanEdgeSection *currentSection,
                       const uint32_t edgeSectionId);
    void processEdgeSection(const uint32_t edgeSectionId,
                            const float weight);
};

}

#endif // NODECLUSTER_H
