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
    KyoChanForwardEdgeSection* getEdgeBlock();
    bool initEdgeBlocks(const uint32_t numberOfEdgeSections);
    uint32_t addEmptyEdgeSection();
};

}

#endif // EDGECLUSTER_H
