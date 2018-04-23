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
                const uint32_t numberOfNodes);
    NodeCluster(const ClusterID clusterId,
                const std::string directoryPath);
    ~NodeCluster();

    uint32_t getNumberOfNodeBlocks() const;
    uint16_t getNumberOfNodes() const;

    void addOutputCluster(const ClusterID clusterId,
                          const uint32_t fakeAxonId,
                          const uint16_t nodeId);

    KyoChanNode* getNodeBlock();

    bool initNodeBlocks(uint16_t numberOfNodes);
};

}

#endif // NODECLUSTER_H
