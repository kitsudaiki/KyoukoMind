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

#include "emptyCluster.h"
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>

namespace PerformanceIO {
class DataBuffer;
}
namespace KyoukoMind
{

class EdgeCluster : public EmptyCluster
{

public:
    EdgeCluster(const ClusterID clusterId,
                const std::string directoryPath,
                MessageController *controller);

    EdgeCluster(const ClusterID clusterId,
                const uint8_t clusterType,
                const std::string directoryPath,
                MessageController *controller);

    ~EdgeCluster();

    uint32_t getNumberOfAxons() const;

    uint32_t getNumberOfAxonBlocks() const;
    uint32_t getNumberOfEdgeBlocks() const;

    uint32_t getNumberOfPendingEdges() const;
    uint32_t getNumberOfMaxPendingEdges() const;

    KyoChanAxon* getAxonBlock();
    KyoChanEdgeSection* getEdgeBlock();
    KyoChanPendingEdgeSectionBig* getPendingEdges();

    bool initAxonBlocks(uint32_t numberOfAxons);
    bool initEdgeBlocks(uint32_t numberOfEdgeSections);

    bool addEdge(const uint32_t axonId, const KyoChanEdge &newEdge);

    void syncEdgeSections(uint32_t startSection = 0,
                          uint32_t endSection = 0);

private:
    KyoChanPendingEdgeSectionBig m_pendingEdges;

    bool isEdgeSectionFull(const uint32_t edgeSectionPos);
    bool addEmptyEdgeSection(const uint32_t axonId);

};

}

#endif // EDGECLUSTER_H
