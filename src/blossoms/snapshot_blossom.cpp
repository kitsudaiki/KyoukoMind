/**
 * @file        snapshot_blossom.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2018 Tobias Anker
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

#include "snapshot_blossom.h"

#include <libKitsunemimiCommon/threading/thread.h>

#include <libKitsunemimiPersistence/logger/logger.h>
#include <libKitsunemimiPersistence/files/text_file.h>

#include <import_export/obj_converter.h>
#include <core/network_manager.h>
#include <core/processing/cpu/edge_processing.h>
#include <core/objects/segment.h>
#include <kyouko_root.h>

using namespace Kitsunemimi::Sakura;

SnapshotBlossom::SnapshotBlossom()
{
}

bool
SnapshotBlossom::runTask(BlossomLeaf &,
                         std::string &errorMessage)
{
    LOG_DEBUG("snapshot network");

    KyoukoRoot::m_root->m_networkManager->initBlockThread();
    sleep(1);

    std::string snapshot = "";

    //clearAllEdgeSections();

    convertNetworkToString(snapshot);
    const bool ret = Kitsunemimi::Persistence::writeFile("/tmp/snapshot.obj",
                                                         snapshot,
                                                         errorMessage,
                                                         true);

    KyoukoRoot::m_root->m_networkManager->continueThread();

    return ret;
}
