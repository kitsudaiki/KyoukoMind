/**
 *  @file    clusterHandler.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTERHANDLER_H
#define CLUSTERHANDLER_H

#include <common.h>

namespace KyoukoMind
{
class Cluster;
class EdgeCluster;
class ClusterQueue;
class GlobalValuesHandler;

using Kitsune::Json::JsonItem;
using Kitsune::Json::JsonObject;
using Kitsune::Json::JsonArray;
using Kitsune::Json::JsonValue;

class ClusterHandler
{
public:
    ClusterHandler();
    ~ClusterHandler();

    // add
    bool addCluster(const ClusterID clusterId,
                    Cluster* cluster,
                    bool addToQueue = true);

    // getter
    Cluster* getCluster(const ClusterID clusterId);
    Cluster* getClusterByIndex(const uint32_t index);
    uint64_t getNumberOfCluster() const;
    ClusterQueue* getClusterQueue() const;
    GlobalValuesHandler* getGlobalValuesHandler() const;

    // delete
    bool deleteCluster(const ClusterID clusterId);
    void clearAllCluster();

    bool setNewConnection(const ClusterID targetClusterId,
                          const uint8_t targetSide,
                          const ClusterID sourceClusterId,
                          const bool bidirect = true);

    // extract
    JsonItem extractNode(const uint32_t clusterId,
                         const uint16_t nodeId);
    JsonArray* getEdgeSectionInfos(EdgeCluster* edgeCluster,
                                   const uint32_t sectionId);


private:
    std::map<ClusterID, Cluster*> m_allClusters;
    ClusterQueue* m_clusterQueue = nullptr;
    GlobalValuesHandler* m_globalValuesHandler = nullptr;
};

}

#endif // CLUSTERHANDLER_H
