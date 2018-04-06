#ifndef CLUSTERINITILIZER_H
#define CLUSTERINITILIZER_H

#include <common.h>
#include <core/structs/clusterMeta.h>

namespace KyoukoMind
{

class Cluster;
class ClusterHandler;
class MessageController;

class ClusterInitilizer
{
public:
    ClusterInitilizer(std::vector<std::vector<MetaDataEntry>> *networkMetaStructure,
                      uint32_t networkDimensionX,
                      uint32_t networkDimensionY,
                      const std::string directoryPath,
                      ClusterHandler *clusterHandler,
                      MessageController *messageController);

    bool createNetwork();

private:
    std::vector<std::vector<MetaDataEntry>>* m_networkMetaStructure = nullptr;
    uint32_t m_networkDimensionX = 0;
    uint32_t m_networkDimensionY = 0;
    std::string m_directoryPath = "";
    ClusterHandler* m_clusterHandler = nullptr;
    MessageController* m_messageController = nullptr;

    bool addCluster(const uint32_t x,
                    const uint32_t y,
                    const uint32_t nodeNumberPerCluster);
    bool addNeighbors(const uint32_t x,
                      const uint32_t y,
                      Cluster* cluster);
    std::pair<uint32_t, uint32_t> getNext(const uint32_t x,
                                          const uint32_t y,
                                          const uint8_t side);
    uint32_t getDistantToNextNodeCluster(const uint32_t x,
                                         const uint32_t y,
                                         const uint8_t side);
};

}

#endif // CLUSTERINITILIZER_H
