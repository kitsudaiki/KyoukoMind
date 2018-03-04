#include <KyoChanNetwork.h>
#include <core/networkManager.h>


namespace KyoukoMind
{

Config* KyoukoNetwork::m_config = nullptr;
Persistence::Logger* KyoukoNetwork::m_logger = nullptr;
KyoukoMind::Database* KyoukoNetwork::m_db = nullptr;

/**
 * @brief KyoChanNetwork::KyoChanNetwork
 * @param configPath
 */
KyoukoNetwork::KyoukoNetwork(const QString &configPath)
{
    m_config = new Config(configPath);

    m_netThreadManager = new NetworkManager();

    bool ok = false;
    //TODO: ok mit übergeben und überprüfen
    m_db = new KyoukoMind::Database(m_config);
    Logger initLogger;
    m_logger = initLogger.initLogger(&ok);
}

}
