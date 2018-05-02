/**
 *  @file    networkInitializer.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef NETWORKINITIALIZER_H
#define NETWORKINITIALIZER_H

#include <common.h>
#include <core/structs/clusterMeta.h>

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
    // initial values from constructor
    std::string m_filePath = "";
    std::string m_directoryPath = "";
    ClusterHandler* m_clusterHandler = nullptr;
    MessageController* m_messageController = nullptr;

    // meta-data of the network
    std::vector<std::vector<InitMetaDataEntry>> m_networkMetaStructure;
    uint32_t m_networkDimensions[2];
    ClusterID m_idCounter = 0;

    // process initial file
    bool getNetworkMetaStructure();
};

}

#endif // NETWORKINITIALIZER_H
