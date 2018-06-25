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

#include <core/clustering/cluster/cluster.h>
#include <core/clustering/cluster/edgeCluster.h>
#include <core/clustering/cluster/nodeCluster.h>

namespace KyoukoMind
{

/**
 * @brief AxonInitializer::AxonInitializer
 * @param networkMetaStructure
 * @param networkDimensionX
 * @param networkDimensionY
 */
AxonInitializer::AxonInitializer(std::vector<std::vector<InitMetaDataEntry> > *networkMetaStructure,
                                 const uint32_t networkDimensionX,
                                 const uint32_t networkDimensionY)
{
    m_networkMetaStructure = networkMetaStructure;
    m_networkDimensionX = networkDimensionX;
    m_networkDimensionY = networkDimensionY;
}


/**
 * @brief AxonInitializer::createAxons
 * @return
 */
bool AxonInitializer::createAxons()
{
    // calculate number of axons per cluster
    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++) {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++) {

            // check cluster-type
            Cluster* cluster = (*m_networkMetaStructure)[x][y].nodeCluster;
            if(cluster == nullptr) {
                continue;
            }
            if(cluster->getClusterType() == NODE_CLUSTER)
            {
                // get node-cluster
                NodeCluster* nodeCluster = static_cast<NodeCluster*>(cluster);
                uint32_t nodeNumberPerCluster = nodeCluster->getNumberOfNodes();
                KyoChanNode* nodes = nodeCluster->getNodeBlock();

                // iterate over all nodes of the cluster and create an axon for each node
                for(uint16_t nodeNumber = 0; nodeNumber < nodeNumberPerCluster; nodeNumber++)
                {
                    // create new axon
                    uint32_t axonId = (*m_networkMetaStructure)[x][y].numberOfAxons;
                    NewAxon newAxon = getNextAxonPathStep(x, y, 0, 8, 0);

                    // update values of the cluster and the node
                    (*m_networkMetaStructure)[newAxon.targetX][newAxon.targetY].numberOfAxons++;
                    nodes[nodeNumber].targetClusterPath = newAxon.targetPath;
                    nodes[nodeNumber].targetAxonId = axonId;
                }
            }
        }
    }

    // add the calculated number of axons to all clusters
    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++) {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++) {

            Cluster* cluster = (*m_networkMetaStructure)[x][y].cluster;
            if(cluster == nullptr) {
                continue;
            }
            // check cluster-type
            if(cluster->getClusterType() == EDGE_CLUSTER)
            {
                // add the axon-number to the specific cluster
                EdgeCluster* edgeCluster = static_cast<EdgeCluster*>(cluster);
                edgeCluster->initForwardEdgeSectionBlocks((*m_networkMetaStructure)[x][y].numberOfAxons);
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
AxonInitializer::NewAxon AxonInitializer::getNextAxonPathStep(const uint32_t x,
                                                              const uint32_t y,
                                                              const uint8_t inputSide,
                                                              const uint32_t currentPath,
                                                              const uint32_t currentStep)
{
    // check if go to next
    bool goToNext = false;
    if(rand() % 100 < POSSIBLE_NEXT_AXON_STEP) {
         goToNext = true;
    }

    // at least one axon-step
    if(inputSide == 0) {
        goToNext = true;
    }

    // return the current values if no next or path long enough
    if(goToNext == false || currentStep == 8) {
        AxonInitializer::NewAxon result;
        result.targetX = x;
        result.targetY = y;
        result.targetPath = currentPath;
        return result;
    }
    // choose the next cluster
    uint8_t nextSite = chooseNextSide((*m_networkMetaStructure)[x][y].neighbors);

    if(nextSite == 0xFF)
    {
        AxonInitializer::NewAxon result;
        result.targetX = x;
        result.targetY = y;
        result.targetPath = currentPath;
        return result;
    }

    // get the neighbor of the choosen side
    Neighbor choosenOne = (*m_networkMetaStructure)[x][y].neighbors[nextSite];

    // update path
    uint32_t newPath = (currentPath * 17) + nextSite;

    // make next iteration
    return getNextAxonPathStep(choosenOne.targetClusterPos.x,
                               choosenOne.targetClusterPos.y,
                               16 - nextSite,
                               newPath,
                               currentStep+1);
}

/**
 * @brief AxonInitializer::chooseNextSide
 * @param neighbors
 * @return
 */
uint8_t AxonInitializer::chooseNextSide(Neighbor *neighbors)
{
    std::vector<uint8_t> sideOrder = {2, 3, 4, 14, 13, 12};
    std::vector<uint8_t> availableSides;

    for(uint8_t i = 0; i < sideOrder.size(); i++)
    {
        if(neighbors[sideOrder[i]].targetClusterId != UNINIT_STATE) {
            availableSides.push_back(sideOrder[i]);
        }
    }

    if(availableSides.size() != 0) {
        return availableSides[rand() % availableSides.size()];
    }

    return 0xFF;
}

}
