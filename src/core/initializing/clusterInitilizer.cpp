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

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

#include <core/cluster/clusterHandler.h>
#include <core/processing/processingUnitHandler.h>
#include <core/messaging/messageController.h>

namespace KyoukoMind
{

ClusterInitilizer::ClusterInitilizer(std::vector<std::vector<MetaDataEntry> > *networkMetaStructure,
                                     uint32_t networkDimensionX,
                                     uint32_t networkDimensionY,
                                     const std::string directoryPath,
                                     ClusterHandler *clusterHandler,
                                     MessageController* messageController)
{
    m_networkMetaStructure = networkMetaStructure;
    m_networkDimensionX = networkDimensionX;
    m_networkDimensionY = networkDimensionY;
    m_directoryPath = directoryPath;
    m_clusterHandler = clusterHandler;
    m_messageController = messageController;
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
        std::cout<<"------"<<std::endl;
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++)
        {
            addCluster(x, y, nodeNumberPerCluster);
        }
    }
    std::cout<<"------"<<std::endl;
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
    Cluster* cluster = nullptr;
    // create cluster
    switch ((int)(*m_networkMetaStructure)[x][y].type) {
        case 0:
            cluster = new EmptyCluster((*m_networkMetaStructure)[x][y].clusterId,
                                       m_directoryPath,
                                       m_messageController);
            std::cout<<x<<"  "<<y<<"  Empty"<<std::endl;
            break;
        case 1:
            cluster = new EdgeCluster((*m_networkMetaStructure)[x][y].clusterId,
                                      m_directoryPath,
                                      m_messageController);
            std::cout<<x<<"  "<<y<<"  edge"<<std::endl;
            break;
        case 2:
            cluster = new NodeCluster((*m_networkMetaStructure)[x][y].clusterId,
                                      m_directoryPath,
                                      nodeNumberPerCluster,
                                      m_messageController);
            std::cout<<x<<"  "<<y<<"  node"<<std::endl;
            break;
        default:
            return false;
    }
    std::cout<<"id: "<<(*m_networkMetaStructure)[x][y].clusterId<<std::endl;
    addNeighbors(x, y, cluster);
    m_clusterHandler->addCluster((*m_networkMetaStructure)[x][y].clusterId, cluster);
    (*m_networkMetaStructure)[x][y].cluster = cluster;
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
    std::vector<uint8_t> sideOrder = {0,1,2,9,8,7};
    for(uint8_t i = 0; i < sideOrder.size(); i++)
    {
        uint8_t side = sideOrder[i];
        // get new neighbor
        std::pair<uint32_t, uint32_t> next = getNext(x, y, side);

        // set the values in the neighbor-struct
        Neighbor tempNeighbor;
        tempNeighbor.targetClusterId = (*m_networkMetaStructure)[next.first][next.second].clusterId;
        tempNeighbor.neighborType = (*m_networkMetaStructure)[next.first][next.second].type;
        tempNeighbor.distantToNextNodeCluster = getDistantToNextNodeCluster(x, y, side);
        tempNeighbor.targetClusterPos.x = x;
        tempNeighbor.targetClusterPos.y = y;
        tempNeighbor.side = side;

        // add new neighbor
        cluster->addNeighbor(side, tempNeighbor);
        (*m_networkMetaStructure)[next.first][next.second].neighbors[side] = tempNeighbor;
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

    // TODO: max abort-distance
    for(uint32_t distance = 1; distance < (*m_networkMetaStructure).size(); distance++)
    {
        if((*m_networkMetaStructure)[next.first][next.second].type == (uint8_t)NODECLUSTER) {
            return distance;
        }
        if((*m_networkMetaStructure)[next.first][next.second].type == (uint8_t)EMPTYCLUSTER) {
            return 0;
        }
        next = getNext(next.first, next.second, side);
    }
    return 0;
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
    case 0:
        {
            result.first = x;
            if(y == 0) {
                result.second = m_networkDimensionY - 1;
            } else {
                result.second = y - 1;
            }
            break;
        }
    case 1:
        {
            result.first = (x + 1) % m_networkDimensionX;
            if(y % 2 == 1) {
                result.second = y;
            } else {
                if(y == 0) {
                    result.second = m_networkDimensionY - 1;
                } else {
                    result.second = y - 1;
                }
            }
            break;
        }
    case 2:
        {
            result.first = (x + 1) % m_networkDimensionX;
            if(y % 2 == 1) {
                result.second = (y + 1) % m_networkDimensionY;
            } else {
                result.second = y;
            }
            break;
        }
    case 9:
        {
            result.first = x;
            result.second = (y + 1) % m_networkDimensionY;
            break;
        }
    case 8:
        {
            if(x == 0) {
                result.first = m_networkDimensionX - 1;
            } else {
                result.first = x - 1;
            }
            if(y % 2 == 1) {
                result.second = (y + 1) % m_networkDimensionY;
            } else {
                result.second = y;
            }
            break;
        }
    case 7:
        {
            if(x == 0) {
                result.first = m_networkDimensionX - 1;
            } else {
                result.first = x - 1;
            }
            if(y % 2 == 1) {
                result.second = y;
            } else {
                if(y == 0) {
                    result.second = m_networkDimensionY - 1;
                } else {
                    result.second = y - 1;
                }
            }
            break;
        }
    default:
        break;
    }
    return result;
}

}
