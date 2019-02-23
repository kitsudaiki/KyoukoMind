/**
 *  @file    clusterHandler.cpp
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#include <core/clustering/clusterHandler.h>
#include <core/clustering/clusterQueue.h>
#include <core/clustering/cluster/cluster.h>
#include <core/clustering/globalValuesHandler.h>

#include <core/structs/kyochanNodes.h>
#include <core/structs/kyochanEdges.h>
#include <core/clustering/cluster/nodeCluster.h>
#include <core/clustering/cluster/edgeCluster.h>

namespace KyoukoMind
{

/**
 * @brief ClusterManager::ClusterManager
 */
ClusterHandler::ClusterHandler()
{
    m_globalValuesHandler = new GlobalValuesHandler();
    m_clusterQueue = new ClusterQueue(m_globalValuesHandler);
}

/**
 * @brief ClusterManager::~ClusterManager
 */
ClusterHandler::~ClusterHandler()
{
    delete m_clusterQueue;
    clearAllCluster();
}

/**
 * @brief ClusterManager::addCluster
 * @param clusterId
 * @param cluster
 * @return
 */
bool
ClusterHandler::addCluster(const ClusterID clusterId,
                           Cluster* cluster,
                           bool addToQueue)
{
    if(m_allClusters.find(clusterId) != m_allClusters.end()) {
        return false;
    }
    cluster->setGlobalValuesHandler(m_globalValuesHandler);
    m_allClusters.insert(std::pair<ClusterID, Cluster*>(clusterId, cluster));

    if(addToQueue) {
        // TODO: remove from handler when this call returns false
        return m_clusterQueue->addCluster(cluster);
    }
    return true;
}

/**
 * @brief ClusterManager::getCluster
 * @param clusterId
 * @return
 */
Cluster*
ClusterHandler::getCluster(const ClusterID clusterId)
{
    std::map<ClusterID, Cluster*>::iterator it;
    it = m_allClusters.find(clusterId);
    if(it != m_allClusters.end()) {
        return it->second;
    }
    return nullptr;
}

/**
 * @brief ClusterHandler::getClusterByPos
 * @param clusterPos
 * @return
 */
Cluster*
ClusterHandler::getClusterByIndex(const uint32_t index)
{
    if(index >= m_allClusters.size()) {
        return nullptr;
    }
    uint32_t counter = 0;
    std::map<ClusterID, Cluster*>::iterator it;
    for(it = m_allClusters.begin(); it != m_allClusters.end(); ++it) {
        if(counter == index) {
            return it->second;
        }
        counter++;
    }
    return nullptr;
}

/**
 * @brief ClusterHandler::getNumberOfCluster
 * @return
 */
uint64_t
ClusterHandler::getNumberOfCluster() const
{
    return m_allClusters.size();
}

/**
 * @brief ClusterManager::deleteCluster
 * @param clusterId
 * @return
 */
bool
ClusterHandler::deleteCluster(const ClusterID clusterId)
{
    std::map<ClusterID, Cluster*>::iterator it;
    it = m_allClusters.find(clusterId);
    if(it != m_allClusters.end()) {
        m_allClusters.erase(it);
        return true;
    }
    return false;
}

/**
 * @brief ClusterManager::clearAllCluster
 */
void
ClusterHandler::clearAllCluster()
{
    std::map<ClusterID, Cluster*>::iterator it;
    for(it = m_allClusters.begin(); it != m_allClusters.end(); ++it) {
        Cluster* tempCluster = it->second;
        delete tempCluster;
    }
    m_allClusters.clear();
    m_clusterQueue->clearQueue();
}

/**
 * @brief ClusterHandler::setNewConnection
 * @param targetClusterId
 * @param sourceSide
 * @param buffer
 * @return
 */
bool
ClusterHandler::setNewConnection(const ClusterID targetClusterId,
                                 const uint8_t targetSide,
                                 const ClusterID sourceClusterId,
                                 const bool bidirect)
{
    Cluster* targetCluster = getCluster(targetClusterId);
    Cluster* sourceCluster = getCluster(sourceClusterId);

    if(targetCluster != nullptr && sourceCluster != nullptr)
    {
        sourceCluster->setNewConnection(
                    16 - targetSide,
                    targetCluster->getIncomingMessageBuffer(targetSide));

        if(bidirect) {
            targetCluster->setNewConnection(
                        targetSide,
                        sourceCluster->getIncomingMessageBuffer(16 - targetSide));
        }
        return true;
    }
    return false;
}

/**
 * @brief ClusterHandler::getClusterQueue
 * @return
 */
ClusterQueue*
ClusterHandler::getClusterQueue() const
{
    return m_clusterQueue;
}

/**
 * @brief ClusterHandler::getGlobalValuesHandler
 * @return
 */
GlobalValuesHandler*
ClusterHandler::getGlobalValuesHandler() const
{
    return m_globalValuesHandler;
}

/**
 * @brief ClusterHandler::extractNode
 * @param clusterId
 * @param nodeId
 * @return
 */
JsonItem ClusterHandler::extractNode(const uint32_t clusterId,
                                     const uint16_t nodeId)
{
    JsonObject* result = new JsonObject();
    NodeCluster* sourceCluster = static_cast<NodeCluster*>(getCluster(clusterId));
    KyoChanNode sourceNode = (sourceCluster->getNodeBlock())[nodeId];

    Cluster* tempCluster = sourceCluster;
    uint32_t nextId = nodeId;

    JsonArray* array = new JsonArray();
    while(sourceNode.targetClusterPath != 0)
    {
        const uint8_t side = sourceNode.targetClusterPath % 32;
        sourceNode.targetClusterPath /= 32;

        nextId = tempCluster->getNeighborId(side);
        array->append(new JsonValue((int)side));
        tempCluster = getCluster(nextId);
    }

    EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(getCluster(nextId));

    result->insert("axon", array);
    result->insert("pathes", getEdgeSectionInfos(edgeCluster, sourceNode.targetAxonId));

    return JsonItem(result);
}

/**
 * @brief ClusterHandler::getEdgeSectionInfos
 * @param edgeCluster
 * @return
 */
JsonArray*
ClusterHandler::getEdgeSectionInfos(EdgeCluster *edgeCluster, const uint32_t sectionId)
{
    JsonArray* result = new JsonArray();
    KyoChanForwardEdgeSection section = (edgeCluster->getForwardEdgeSectionBlock())[sectionId];

    for(int i = 2; i < 16; i++)
    {
        if(section.forwardEdges[i].targetId != UNINIT_STATE_32)
        {
            uint32_t nextSectionId = section.forwardEdges[i].targetId;
            uint32_t nextClusterId = edgeCluster->getNeighbors()[i].targetClusterId;

            Cluster* nextCluster = getCluster(nextClusterId);

            JsonObject* obj = new JsonObject();
            obj->insert("side", new JsonValue(i));

            if(nextCluster->getClusterType() == EDGE_CLUSTER) {
                obj->insert("next", getEdgeSectionInfos(static_cast<EdgeCluster*>(nextCluster), nextSectionId));
            }
            result->append(obj);
        }
    }
    return result;
}

}
