/**
 * @file        kyouko_root.h
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

#ifndef KYOUKOMIND_KYOUKO_ROOT_H
#define KYOUKOMIND_KYOUKO_ROOT_H

#include <common.h>

class ClusterHandler;
class SegmentQueue;
class ProcessingUnitHandler;

class KyoukoRoot
{

public:
    KyoukoRoot();
    ~KyoukoRoot();

    static KyoukoRoot* m_root;
    static ClusterHandler* m_clusterHandler;
    static uint32_t* m_randomValues;
    static SegmentQueue* m_segmentQueue;
    static ProcessingUnitHandler* m_processingUnitHandler;

    bool initializeSakuraFiles(Kitsunemimi::ErrorContainer &error);
    const std::string initCluster(const std::string &filePath,
                                  Kitsunemimi::ErrorContainer &error);

private:
    uint32_t m_serverId = 0;
};

#endif //KYOUKOMIND_KYOUKO_ROOT_H
