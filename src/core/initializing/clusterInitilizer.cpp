/**
 *  @file    clusterInitializer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "clusterInitilizer.h"
#include <kyochanNetwork.h>

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

#include <core/clustering/clusterHandler.h>
#include <core/processing/processingUnitHandler.h>

namespace KyoukoMind
{

/**
 * @brief ClusterInitilizer::ClusterInitilizer
 * @param networkMetaStructure
 * @param networkDimensionX
 * @param networkDimensionY
 * @param directoryPath
 * @param clusterHandler
 */
ClusterInitilizer::ClusterInitilizer(std::vector<std::vector<InitMetaDataEntry> > *networkMetaStructure,
                                     const uint32_t networkDimensionX,
                                     const uint32_t networkDimensionY,
                                     const std::string directoryPath,
                                     ClusterHandler *clusterHandler)
{
    m_networkMetaStructure = networkMetaStructure;
    m_networkDimensionX = networkDimensionX;
    m_networkDimensionY = networkDimensionY;
    m_directoryPath = directoryPath;
    m_clusterHandler = clusterHandler;
}

/**
 * @brief ClusterInitilizer::createNetwork
 * @return
 */
bool ClusterInitilizer::createNetwork()
{
    bool ok = false;
    uint32_t nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);

    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++)
        {
            addCluster(x, y, nodeNumberPerCluster);
        }
    }

    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++)
        {
            std::vector<uint8_t> sideOrder = {2,3,4,14,13,12};
            for(uint8_t i = 0; i < sideOrder.size(); i++)
            {
                const uint8_t side = sideOrder[i];
                if((*m_networkMetaStructure)[x][y].cluster != nullptr) {
                    m_clusterHandler->setNewConnection(
                                (*m_networkMetaStructure)[x][y].neighbors[side].targetClusterId,
                                (*m_networkMetaStructure)[x][y].neighbors[side].targetSide,
                                (*m_networkMetaStructure)[x][y].cluster->getClusterId());
                }
            }

            if((*m_networkMetaStructure)[x][y].nodeCluster != nullptr)
            {
                m_clusterHandler->setNewConnection(
                            (*m_networkMetaStructure)[x][y].nodeCluster->getClusterId(),
                            8,
                            (*m_networkMetaStructure)[x][y].cluster->getClusterId());
            }
        }
    }
    return true;
}

/**
 * @brief ClusterInitilizer::addCluster
 * @param x
 * @param y
 * @param nodeNumberPerCluster
 * @return
 */
bool ClusterInitilizer::addCluster(const uint32_t x,
                                   const uint32_t y,
                                   const uint32_t nodeNumberPerCluster)
{
    // create cluster
    switch ((int)(*m_networkMetaStructure)[x][y].type) {
        case 1:
            return true;
        case 2:
        {
            Cluster*  cluster = new EdgeCluster((*m_networkMetaStructure)[x][y].clusterId,
                                                m_directoryPath);
            addNeighbors(x, y, cluster);

            m_clusterHandler->addCluster((*m_networkMetaStructure)[x][y].clusterId, cluster);
            (*m_networkMetaStructure)[x][y].cluster = cluster;
            break;
        }
        case 3:
        {
            Cluster*  edgeCluster = new EdgeCluster((*m_networkMetaStructure)[x][y].clusterId,
                                                    m_directoryPath);
            addNeighbors(x, y, edgeCluster);

            edgeCluster->setNeighbor(8, (*m_networkMetaStructure)[x][y].clusterId+1);

            m_clusterHandler->addCluster((*m_networkMetaStructure)[x][y].clusterId, edgeCluster);
            (*m_networkMetaStructure)[x][y].cluster = edgeCluster;

            Cluster*  nodeCluster = new NodeCluster((*m_networkMetaStructure)[x][y].clusterId+1,
                                                    m_directoryPath,
                                                    nodeNumberPerCluster);

            nodeCluster->setNeighbor(8, (*m_networkMetaStructure)[x][y].clusterId);

            m_clusterHandler->addCluster((*m_networkMetaStructure)[x][y].clusterId+1, nodeCluster);
            (*m_networkMetaStructure)[x][y].nodeCluster = nodeCluster;
            break;
        }
        default:
            return false;
    }
    return true;
}

/**
 * @brief ClusterInitilizer::addNeighbors
 * @param x
 * @param y
 * @param cluster
 * @return
 */
bool ClusterInitilizer::addNeighbors(const uint32_t x, const uint32_t y, Cluster* cluster)
{
    std::vector<uint8_t> sideOrder = {2,3,4,14,13,12};
    for(uint8_t i = 0; i < sideOrder.size(); i++)
    {
        uint8_t side = sideOrder[i];
        // get new neighbor
        std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

        // set the values in the neighbor-struct
        Neighbor tempNeighbor;
        if((*m_networkMetaStructure)[next.first][next.second].type != EMPTY_CLUSTER)
        {
            tempNeighbor.targetClusterId = (*m_networkMetaStructure)[next.first][next.second].clusterId;
            tempNeighbor.targetSide = 16 - side;
            tempNeighbor.targetClusterPos.x = next.first;
            tempNeighbor.targetClusterPos.y = next.second;
            // add new neighbor
            cluster->setNeighbor(side, (*m_networkMetaStructure)[next.first][next.second].clusterId);

            tempNeighbor.incomBuffer = cluster->getIncomingMessageBuffer(side);
            tempNeighbor.outgoBuffer = cluster->getOutgoingMessageBuffer(side);
        }

        (*m_networkMetaStructure)[x][y].neighbors[side] = tempNeighbor;
    }

    return true;
}

/**
 * @brief NetworkInitializer::getDistantToNextNodeCluster
 * @param x
 * @param y
 * @param side
 * @return
 */
uint32_t ClusterInitilizer::getDistantToNextNodeCluster(const uint32_t x,
                                                        const uint32_t y,
                                                        const uint8_t side)
{
    std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

    uint32_t maxDistance = (*m_networkMetaStructure).size();
    if(maxDistance > MAX_DISTANCE-1) {
        maxDistance = MAX_DISTANCE-1;
    }
    for(uint32_t distance = 1; distance < (*m_networkMetaStructure).size(); distance++)
    {
        if((*m_networkMetaStructure)[next.first][next.second].type == (uint8_t)NODE_CLUSTER) {
            return distance;
        }
        if((*m_networkMetaStructure)[next.first][next.second].type == (uint8_t)EMPTY_CLUSTER) {
            return MAX_DISTANCE;
        }
        next = getNext(next.first, next.second, side);
    }
    return MAX_DISTANCE;
}

/**
 * @brief ClusterInitilizer::getNext
 * @param x
 * @param y
 * @param side
 * @return
 */
std::pair<uint32_t, uint32_t> ClusterInitilizer::getNext(const uint32_t x,
                                                         const uint32_t y,
                                                         const uint8_t side)
{
    std::pair<uint32_t, uint32_t> result;
    switch (side) {
    case 2:
        {
            result.first = x - 1;
            if(x % 2 == 1) {
                result.second = y + 1;
            } else {
                result.second = y;
            }
            break;
        }
    case 3:
        {
            result.first = x;
            result.second = y + 1;
            break;
        }
    case 4:
        {
            result.first = x + 1;
            if(x % 2 == 1) {
                result.second = y + 1;
            } else {
                result.second = y;
            }
            break;
        }
    case 14:
        {
            result.first = x + 1;
            if(x % 2 == 1) {
                result.second = y;
            } else {
                result.second = y - 1;
            }
            break;
        }
    case 13:
        {
            result.first = x;
            result.second = y - 1;
            break;
        }
    case 12:
        {
            result.first = x - 1;
            if(x % 2 == 1) {
                result.second = y;
            } else {
                result.second = y - 1;
            }
            break;
        }
    default:
        break;
    }
    return result;
}

}
