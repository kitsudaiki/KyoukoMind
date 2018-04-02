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
                       ClusterHandler *clusterHandler,
                       MessageController* messageController);

    bool initNetwork();


    std::vector<std::string> splitString(const std::string &s, char delim) {
        std::stringstream ss(s);
        std::string item;
        std::vector<std::string> tokens;
        while(std::getline(ss, item, delim)) {
            tokens.push_back(item);
        }
        return tokens;
    }

    void removeEmptyStrings(std::vector<std::string>& strings)
    {
        std::vector<std::string>::iterator it = std::remove_if(
                  strings.begin(),
                  strings.end(),
                  std::mem_fun_ref(&std::string::empty));
        // erase the removed elements
        strings.erase(it, strings.end());
    }

private:
    struct MetaDataEntry {
        uint8_t type = 0;
        ClusterID clusterId = 0;
        Neighbor neighbors[10];
    };

    std::string m_filePath = "";
    std::string m_directoryPath = "";
    ClusterHandler* m_clusterHandler = nullptr;
    MessageController* m_messageController = nullptr;
    ClusterID m_idCounter = 1;

    std::vector<std::vector<MetaDataEntry>> m_networkMetaStructure;
    uint32_t m_networkDimensions[2];

    bool getNetworkMetaStructure();
    bool createNetwork();
    bool addCluster(const uint32_t x, const uint32_t y, const uint32_t nodeNumberPerCluster);
    bool addNeighbors(const uint32_t x, const uint32_t y, Cluster* cluster);
    bool createAxons();

    std::pair<uint32_t, uint32_t> getNext(const uint32_t x, const uint32_t y, const uint8_t side);
    uint32_t getDistantToNextNodeCluster(const uint32_t x, const uint32_t y, const uint8_t side);
};

}

#endif // NETWORKINITIALIZER_H
