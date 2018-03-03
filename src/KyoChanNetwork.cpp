#include <KyoChanNetwork.h>
#include <core/networkManager.h>


namespace KyoChan_Network
{

Config* KyoChanNetwork::m_config = nullptr;
Persistence::Logger* KyoChanNetwork::m_logger = nullptr;
KyoChan_Network::DatabaseController* KyoChanNetwork::m_dbCon = nullptr;

/**
 * @brief KyoChanNetwork::KyoChanNetwork
 * @param configPath
 */
KyoChanNetwork::KyoChanNetwork(const QString &configPath)
{
    m_config = new Config(configPath);

    m_netThreadManager = new NetworkManager();

    bool ok = false;
    //TODO: ok mit übergeben und überprüfen
    m_dbCon = new KyoChan_Network::DatabaseController(m_config);
    InitLogger initLogger;
    m_logger = initLogger.initLogger(&ok);
}

}
