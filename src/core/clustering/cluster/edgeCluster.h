/**
 *  @file    edgeCluster.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"
#include <tests/core/clustering/cluster/edgeClusterTest.h>

namespace KyoukoMind
{

class EdgeCluster : public Cluster
{

public:
    EdgeCluster(const ClusterID &clusterId,
                const uint32_t x,
                const uint32_t y,
                const std::string directoryPath);

    #ifdef RUN_UNIT_TEST
    friend class EdgeClusterTest;
    KyoChanForwardEdgeSection* getForwardEdgeSectionBlock_public();
    #endif

    // edge-forward-sections
    uint32_t getNumberOfForwardEdgeSectionBlocks() const;
    uint32_t getNumberOfForwardEdgeSections() const;
    bool initForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections);
    uint32_t addEmptyForwardEdgeSection(const uint8_t sourceSide,
                                        const uint32_t sourceId);
    KyoChanForwardEdgeSection* getForwardEdgeSectionBlock();


    // processing
    bool processEdgesCluster();

protected:
    float m_weightMap[17];

    // processing-methods for edge-cluster
    bool checkAndDelete(KyoChanForwardEdgeSection *currentSection,
                        const uint32_t forwardEdgeSectionId);
    void learningForwardEdgeSection(KyoChanForwardEdgeSection *currentSection,
                                    const uint32_t forwardEdgeSectionId,
                                    const float weight);
    void learningEdgeSection(KyoChanEdgeSection *currentSection,
                             const float partitialWeight);
    void processUpdateSetEdge(KyoChanForwardEdgeSection* currentSection,
                              const float updateValue,
                              const uint8_t inititalSide);
    void processUpdateSubEdge(KyoChanForwardEdgeSection* currentSection,
                              float updateValue,
                              const uint8_t inititalSide);
    void processUpdateDeleteEdge(KyoChanForwardEdgeSection* currentSection,
                                 const uint32_t forwardEdgeSectionId,
                                 const uint8_t inititalSide);
    void processUpdateEdge(const uint32_t forwardEdgeSectionId,
                           float updateValue,
                           const uint8_t updateType,
                           const uint8_t inititalSide);
    void processEdgeForwardSection(const uint32_t forwardEdgeSectionId,
                                   const float weight);
    void processAxon(const uint32_t targetId,
                     const uint64_t path,
                     const float weight);
    void processLerningEdge(const uint32_t sourceEdgeSectionId,
                            const float weight,
                            const uint8_t initSide);
    void processPendingEdge(const uint32_t sourceId,
                            const uint8_t sourceSide,
                            const float weight);
};

}

#endif // EDGECLUSTER_H
