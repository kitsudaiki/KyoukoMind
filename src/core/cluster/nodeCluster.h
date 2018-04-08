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
                const uint32_t numberOfNodes,
                MessageController *controller);
    NodeCluster(const ClusterID clusterId,
                const std::string directoryPath,
                MessageController *controller);
    ~NodeCluster();

    uint32_t getNumberOfNodeBlocks() const;

    KyoChanNode* getNodeBlock();

    bool initNodeBlocks(uint16_t numberOfNodes);
};

}

#endif // NODECLUSTER_H
