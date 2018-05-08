/**
 *  @file    nodeCluster.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NODECLUSTER_H
#define NODECLUSTER_H

#include "edgeCluster.h"

namespace KyoukoMind
{

class NodeCluster : public EdgeCluster
{

public:
    NodeCluster(const ClusterID clusterId,
                const std::string directoryPath,
                const uint32_t numberOfNodes = 0);
    ~NodeCluster();

    // nodes
    uint16_t getNumberOfNodeBlocks() const;
    uint16_t getNumberOfNodes() const;
    KyoChanNode* getNodeBlock();
    bool initNodeBlocks(const uint16_t numberOfNodes);

    // edge-sections
    KyoChanEdgeSection* getEdgeBlock();
    bool initEdgeBlocks(const uint32_t numberOfEdgeSections);
    bool addEdge(const uint32_t edgeSectionId, const KyoChanEdge &newEdge);
    uint32_t addEmptyEdgeSection();

    // output
    void addOutputCluster(const ClusterID clusterId,
                          const uint32_t outputAxonId,
                          const uint16_t nodeId);

    // number of active nodes
    uint16_t getNumberOfActiveNodes() const;
    void setNumberOfActiveNodes(const uint16_t &numberOfActiveNodes);

private:
    uint16_t m_numberOfActiveNodes = 0;
};

}

#endif // NODECLUSTER_H
