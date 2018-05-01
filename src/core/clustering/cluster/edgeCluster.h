/**
 *  @file    edgeCluster.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef EDGECLUSTER_H
#define EDGECLUSTER_H

#include "cluster.h"

namespace PerformanceIO {
class DataBuffer;
}
namespace KyoukoMind
{

class EdgeCluster : public Cluster
{

public:
    EdgeCluster(const ClusterID clusterId,
                const std::string directoryPath);

    ~EdgeCluster();

    uint32_t getNumberOfEdgeBlocks() const;
    KyoChanEdgeForwardSection* getEdgeBlock();
    bool initEdgeBlocks(const uint32_t numberOfEdgeSections);
    bool addEmptyEdgeForwardSection(const uint32_t marker);
};

}

#endif // EDGECLUSTER_H
