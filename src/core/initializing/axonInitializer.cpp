/**
 *  @file    axonInitializer.cpp
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#include "axonInitializer.h"
#include <kyochanNetwork.h>

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
    bool ok = false;
    uint32_t nodeNumberPerCluster = KyoukoNetwork::m_config->getNumberOfNodes(&ok);
    // calculate number of axons per cluster
    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++)
    {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++)
        {
            Cluster* cluster = (*m_networkMetaStructure)[x][y].cluster;

            if(cluster->getClusterType() == NODECLUSTER)
            {
                NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
                for(uint16_t nodeNumber = 0; nodeNumber < nodeNumberPerCluster; nodeNumber++)
                {
                    KyoChanNode* nodes = nodeCluster->getNodeBlock();
                    uint32_t axonId = (*m_networkMetaStructure)[x][y].numberOfAxons;
                    (*m_networkMetaStructure)[x][y].numberOfAxons++;
                    std::vector<uint32_t> newAxon = getNextAxonPathStep(x, y, 1, 0, 0);
                    nodes->targetClusterPath = newAxon[2];
                    nodes->targetAxonId = axonId;
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
            if(cluster->getClusterType() == EDGECLUSTER || cluster->getClusterType() == NODECLUSTER) {
                EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
                edgeCluster->initAxonBlocks((*m_networkMetaStructure)[x][y].numberOfAxons);
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
std::vector<uint32_t> AxonInitializer::getNextAxonPathStep(const uint32_t x,
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
    if(goToNext == false || currentStep == 8) {
        (*m_networkMetaStructure)[x][y].numberOfAxons++;
        std::vector<uint32_t> result;
        result.push_back(x);
        result.push_back(y);
        result.push_back(currentPath);
        return result;
    }

    Neighbor* ptr = (*m_networkMetaStructure)[x][y].neighbors;
    uint8_t nextSite = m_chooser->getNextCluster(ptr,
                                                 inputSide,
                                                 true);
    if(nextSite == 0xFF) {
        (*m_networkMetaStructure)[x][y].numberOfAxons++;
        std::vector<uint32_t> result;
        result.push_back(x);
        result.push_back(y);
        result.push_back(currentPath);
        return result;
    }
    Neighbor choosenOne = (*m_networkMetaStructure)[x][y].neighbors[nextSite];

    uint32_t newPath = (currentPath * 0xF) + nextSite;
    return getNextAxonPathStep(choosenOne.targetClusterPos.x,
                               choosenOne.targetClusterPos.y,
                               0xF - nextSite,
                               newPath,
                               currentStep+1);
}

}
