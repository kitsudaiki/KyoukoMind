/**
 * @file        cluster_handler.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "cluster_handler.h"

#include <core/orchestration/cluster_interface.h>

ClusterHandler::ClusterHandler()
{

}

/**
 * @brief ClusterHandler::addCluster
 * @param uuid
 * @param newCluster
 * @return
 */
bool
ClusterHandler::addCluster(const std::string uuid, ClusterInterface *newCluster)
{
    // check if key already exist
    std::map<std::string, ClusterInterface*>::iterator it;
    it = m_allCluster.find(uuid);
    if(it != m_allCluster.end()) {
        return false;
    }

    m_allCluster.insert(std::make_pair(uuid, newCluster));

    return true;
}

/**
 * @brief ClusterHandler::removeCluster
 * @param uuid
 * @return
 */
bool
ClusterHandler::removeCluster(const std::string uuid)
{
    std::map<std::string, ClusterInterface*>::const_iterator it;
    it = m_allCluster.find(uuid);

    if(it != m_allCluster.end())
    {
        if(it->second != nullptr) {
            delete it->second;
        }
        m_allCluster.erase(it);
        return true;
    }

    return false;
}

/**
 * @brief KyoukoRoot::getCluster
 * @param uuid
 * @return
 */
ClusterInterface*
ClusterHandler::getCluster(const std::string uuid)
{
    std::map<std::string, ClusterInterface*>::const_iterator it;
    it = m_allCluster.find(uuid);

    if(it != m_allCluster.end()) {
        return it->second;
    }

    return nullptr;
}
