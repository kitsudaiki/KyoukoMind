/**
 *  @file    axonInitializer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "axonInitializer.h"

#include <core/cluster/cluster.h>
#include <core/cluster/emptyCluster.h>
#include <core/cluster/edgeCluster.h>
#include <core/cluster/nodeCluster.h>

#include <core/processing/nextChooser.h>

namespace KyoukoMind
{

AxonInitializer::AxonInitializer(std::vector<std::vector<MetaDataEntry> > *networkMetaStructure,
                                 uint32_t networkDimensionX,
                                 uint32_t networkDimensionY)
{
    m_networkMetaStructure = networkMetaStructure;
    m_networkDimensionX = networkDimensionX;
    m_networkDimensionY = networkDimensionY;

    m_chooser = new NextChooser();
}


/**
 * @brief AxonInitializer::createAxons
 * @return
 */
bool AxonInitializer::createAxons()
{
    // calculate number of axons per cluster
    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++)
        {
            Cluster* cluster = (*m_networkMetaStructure)[x][y].cluster;
            if(cluster->getClusterType() == NODECLUSTER) {

                std::vector<std::pair<Neighbor, uint8_t>> possibleNexts;
                for(uint8_t side = 0;
                    side < 10;
                    side++) {
                    Neighbor next = (*m_networkMetaStructure)[x][y].neighbors[side];
                    if(next.neighborType == (uint8_t)EDGECLUSTER
                            || next.neighborType == (uint8_t)NODECLUSTER) {
                        possibleNexts.push_back(std::make_pair(next, side));
                    }
                }

                if(possibleNexts.size() == 0) {
                    return false;
                }

            }
        }
    }

    // add the calculated number of axons to all clusters
    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++)
        {
            Cluster* cluster = (*m_networkMetaStructure)[x][y].cluster;
            if(cluster->getClusterType() == EDGECLUSTER) {
                EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
                edgeCluster->initAxonBlocks((*m_networkMetaStructure)[x][y].numberOfAxons);
            }
            if(cluster->getClusterType() == NODECLUSTER) {
                NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
                nodeCluster->initAxonBlocks((*m_networkMetaStructure)[x][y].numberOfAxons);
            }
        }
    }
    return true;
}

/**
 * @brief AxonInitializer::getNextAxonPathStep
 * @param x
 * @param y
 * @param inputSide
 * @param currentPath
 * @param currentStep
 * @return
 */
uint32_t AxonInitializer::getNextAxonPathStep(const uint32_t x,
                                              const uint32_t y,
                                              const uint8_t inputSide,
                                              const uint32_t currentPath,
                                              const uint32_t currentStep)
{
    // check if go to next
    bool goToNext = false;
    if(rand() % 100 <= POSSIBLE_NEXT_AXON_STEP) {
         goToNext = true;
    }
    if(goToNext == false || currentStep == 9) {
        (*m_networkMetaStructure)[x][y].numberOfAxons++;
        return currentPath;
    }

    Neighbor* ptr = (*m_networkMetaStructure)[x][y].neighbors;
    uint8_t nextSite = m_chooser->getNextCluster(ptr,
                                                 inputSide,
                                                 true);
    if(nextSite == 0xFF) {
        (*m_networkMetaStructure)[x][y].numberOfAxons++;
        return currentPath;
    }
    Neighbor choosenOne = (*m_networkMetaStructure)[x][y].neighbors[nextSite];

    uint32_t newPath = (currentPath * 10) + nextSite;
    return getNextAxonPathStep(choosenOne.targetClusterPos.x,
                               choosenOne.targetClusterPos.y,
                               9 - nextSite,
                               newPath,
                               currentStep+1);
}

/**
 * @brief AxonInitializer::calculatePosition
 * @param clusterId
 * @return
 */
std::pair<uint32_t, uint32_t> AxonInitializer::calculatePosition(const uint32_t clusterId)
{
    std::pair<uint32_t, uint32_t> result;
    result.first = clusterId / m_networkDimensionX;
    result.first = clusterId % m_networkDimensionY;
    return result;
}

}
