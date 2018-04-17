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

#include <core/processing/processingThreads/cpu/nextChooser.h>

namespace KyoukoMind
{

/**
 * @brief AxonInitializer::AxonInitializer
 * @param networkMetaStructure
 * @param networkDimensionX
 * @param networkDimensionY
 */
AxonInitializer::AxonInitializer(std::vector<std::vector<MetaDataEntry> > *networkMetaStructure,
                                 const uint32_t networkDimensionX,
                                 const uint32_t networkDimensionY)
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
    for(uint32_t x = 0; x < (*m_networkMetaStructure).size(); x++) {
        for(uint32_t y = 0; y < (*m_networkMetaStructure)[x].size(); y++) {

            // check cluster-type
            Cluster* cluster = (*m_networkMetaStructure)[x][y].cluster;
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
                    AxonInitializer::NewAxon newAxon = getNextAxonPathStep(x, y, 1, 0, 0);

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
            // check cluster-type
            if(cluster->getClusterType() == EDGE_CLUSTER
                    || cluster->getClusterType() == NODE_CLUSTER) {

                // add the axon-number to the specific cluster
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
AxonInitializer::NewAxon AxonInitializer::getNextAxonPathStep(const uint32_t x,
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

    // return the current values if no next or path long enough
    if(goToNext == false || currentStep == 8) {
        AxonInitializer::NewAxon result;
        result.targetX = x;
        result.targetY = y;
        result.targetPath = currentPath;
        return result;
    }

    // choose the next cluster
    uint8_t nextSite = m_chooser->getNextCluster((*m_networkMetaStructure)[x][y].neighbors,
                                                 inputSide,
                                                 true);

    // return the current values if no choise
    if(nextSite == 0xFF) {
        AxonInitializer::NewAxon result;
        result.targetX = x;
        result.targetY = y;
        result.targetPath = currentPath;
        return result;
    }

    // get the neighbor of the choosen side
    Neighbor choosenOne = (*m_networkMetaStructure)[x][y].neighbors[nextSite];

    // update path
    uint32_t newPath = (currentPath * 16) + nextSite;

    // make next iteration
    return getNextAxonPathStep(choosenOne.targetClusterPos.x,
                               choosenOne.targetClusterPos.y,
                               0xF - nextSite,
                               newPath,
                               currentStep+1);
}

}
