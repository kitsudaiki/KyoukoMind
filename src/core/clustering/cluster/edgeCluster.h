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

namespace KyoukoMind
{

class EdgeCluster : public Cluster
{

public:
    EdgeCluster(const ClusterID &clusterId,
                const std::string directoryPath);

    // edge-forward-sections
    uint32_t getNumberOfForwardEdgeSectionBlocks() const;
    uint32_t getNumberOfForwardEdgeSections() const;
    KyoChanForwardEdgeSection* getForwardEdgeSectionBlock();
    bool initForwardEdgeSectionBlocks(const uint32_t numberOfForwardEdgeSections);
    uint32_t addEmptyForwardEdgeSection(const uint8_t sourceSide,
                                        const uint32_t sourceId);
};

}

#endif // EDGECLUSTER_H
