/**
 *  @file    cluster.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef CLUSTER_H
#define CLUSTER_H

#include <common.h>

#include <core/structs/clusterMeta.h>
#include <core/structs/kyochanEdges.h>
#include <core/structs/kyochanNodes.h>
#include <core/structs/globalValues.h>
#include <core/structs/messageContainer.h>

#include <tests/core/clustering/cluster/clusterTest.h>
#include <common/methods.h>

#include <communicationStructs/monitorinContianer.h>
using Kitsune::Chan::Communication::MonitoringMetaData;
using Kitsune::Chan::Communication::MonitoringProcessData;

namespace Kitsune
{
class CommonDataBuffer;
}

namespace KyoukoMind
{
class DataMessage;
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
class GlobalValuesHandler;

class Cluster
{
public:
    Cluster(const ClusterID &clusterId,
            const uint32_t x,
            const uint32_t y,
            const std::string directoryPath,
            const uint32_t staticItemSize,
            const uint32_t dynamicItemSize);
    ~Cluster();

    #ifdef RUN_UNIT_TEST
    friend class ClusterTest;
    #endif

    // metadata
    ClusterMetaData getMetaData() const;
    void getMetaDataFromBuffer();
    void updateMetaData();

    // common getter
    ClusterID getClusterId() const;
    uint8_t getClusterType() const;

    // message-queues
    IncomingMessageBuffer* getIncomingMessageBuffer(const uint8_t side);
    OutgoingMessageBuffer* getOutgoingMessageBuffer(const uint8_t side);
    bool setNewConnection(const uint8_t side, IncomingMessageBuffer* buffer);
    bool setIncomBuffer(const uint8_t side, IncomingMessageBuffer *buffer);
    bool isBufferReady();

    // neighbors
    bool setNeighbor(const uint8_t side, const ClusterID targetClusterId);
    Neighbor *getNeighbors();
    ClusterID getNeighborId(const uint8_t side);

    // cycle
    bool initCycle();
    void finishCycle(NeighborInformation *externalInfo = nullptr);
    bool isCycleInProgress() const;

    // other
    void setGlobalValuesHandler(GlobalValuesHandler* globalValuesHandler);

    #ifdef RUN_UNIT_TEST
    void setTestRandValue(const uint32_t fakeRandVal);
    #endif

    void setLearningOverride(float value);
    bool isInputCluster();

protected:
    Kitsune::CommonDataBuffer* m_clusterDataBuffer = nullptr;
    ClusterMetaData m_metaData;

    float m_learningOverride = 0.5;

    const uint32_t m_staticItemSize;
    const uint32_t m_dynamicItemSize;

    MonitoringMetaData m_monitoringMetaData;
    MonitoringProcessData m_monitoringProcessingData;

    // global values
    GlobalValuesHandler* m_globalValuesHandler = nullptr;
    GlobalValues m_globalValue;

    // cycle specific data
    NeighborInformation m_neighborInfo;
    std::pair<DataMessage*, uint8_t> m_incomingMessages[20];
    uint8_t m_numberOfIncomingMessages = 0;
    bool m_cycleInProgress = false;

    // init blocks
    bool initStaticBlocks(const uint32_t numberOfItems);
    bool initDynamicBlocks(const uint32_t numberOfItems);

    // delete bloks
    uint32_t reserveDynamicItem();
    uint32_t reuseItemPosition();
    bool deleteDynamicItem(const uint32_t itemPos);

    void reportStatus();

    uint32_t m_fakeRandVal = 0;
};

}

#endif // CLUSTER_H
