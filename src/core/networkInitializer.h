#ifndef NETWORKINITIALIZER_H
#define NETWORKINITIALIZER_H

#include <common.h>

namespace KyoukoMind
{
class ClusterHandler;
class Cluster;
class MessageController;

class NetworkInitializer
{
public:
    NetworkInitializer(const std::string filePath,
                       const std::string directoryPath,
                       ClusterHandler *clusterManager,
                       MessageController* messageController);

    bool initNetwork();

private:
    std::string m_filePath = "";
    std::string m_directoryPath = "";
    ClusterHandler* m_clusterManager = nullptr;
    MessageController* m_messageController = nullptr;
    ClusterID m_idCounter = 1;

    std::vector<std::vector<std::pair<uint8_t, ClusterID>>> m_networkMetaStructure;
    uint32_t m_networkDimensions[2];

    bool getNetworkMetaStructure();
    bool createNetwork();
    bool addCluster(const uint32_t x, const uint32_t y, const uint32_t nodeNumberPerCluster);
    bool addNeighbors(const uint32_t x, const uint32_t y, Cluster* cluster);
    bool createAxons();

    std::vector<std::string> splitString(const std::string &s, char delim);
    void removeEmptyStrings(std::vector<std::string> &strings);
};

}

#endif // NETWORKINITIALIZER_H
